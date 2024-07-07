#include <windows.h>
#include <string>
#include <fstream>
#include <vector>
#include <filesystem>
#include <d3d9.h>
#include <d3dx9.h>
#include "toml++/toml.hpp"
#include "nya_commonhooklib.h"

#include "game.h"

void WriteLog(const std::string& str) {
	static auto file = std::ofstream("FlatOut2SkinChanger_gcp.log");

	file << str;
	file << "\n";
	file.flush();
}

PDIRECT3DTEXTURE9 LoadTextureWithDDSCheck(const char* filename) {
	std::ifstream fin(filename, std::ios::in | std::ios::binary );
	if (!fin.is_open()) return nullptr;

	fin.seekg(0, std::ios::end);

	size_t size = fin.tellg();
	if (size <= 0x4C) return nullptr;

	auto data = new char[size];
	fin.seekg(0, std::ios::beg);
	fin.read(data, size);

	if (data[0] != 'D' || data[1] != 'D' || data[2] != 'S') {
		delete[] data;
		return nullptr;
	}

	if (data[0x4C] == 0x18) {
		data[0x4C] = 0x20;
		WriteLog("Loading " + (std::string)filename + " with DDS pixelformat hack");
	}

	PDIRECT3DTEXTURE9 texture;
	auto hr = D3DXCreateTextureFromFileInMemory(g_pd3dDevice, data, size, &texture);
	delete[] data;
	if (hr != S_OK) {
		WriteLog("Failed to load " + (std::string)filename);
		return nullptr;
	}
	return texture;
}

// Simple helper function to load an image into a DX9 texture with common settings
PDIRECT3DTEXTURE9 LoadTexture(const char* filename) {
	if (!std::filesystem::exists(filename)) return nullptr;

	// Load texture from disk
	PDIRECT3DTEXTURE9 texture;
	auto hr = D3DXCreateTextureFromFileA(g_pd3dDevice, filename, &texture);
	if (hr != S_OK) {
		return LoadTextureWithDDSCheck(filename);
	}
	return texture;
}

bool bReplaceAllCarsTextures = false;
int nRefreshKey = VK_INSERT;

Player* pLocalPlayer = nullptr;
std::vector<Player*> aPlayers;

bool bFirstFrameToReplaceTextures = false;
void OnLocalPlayerInitialized() {
	aPlayers.clear();
	bFirstFrameToReplaceTextures = true;
}

uintptr_t CollectLocalPlayerASM_jmp = 0x45DD32;
void __attribute__((naked)) CollectLocalPlayerASM() {
	__asm__ (
		"pushad\n\t"
		"call %2\n\t"
		"popad\n\t"

		"mov %1, eax\n\t"
		"cmp ecx, 7\n\t"
		"mov dword ptr [eax+0x378], 1\n\t"
		"jmp %0\n\t"
			:
			:  "m" (CollectLocalPlayerASM_jmp), "m" (pLocalPlayer), "i" (OnLocalPlayerInitialized)
	);
}

// this hook is just used to collect all player pointers in a race, could prolly be moved somewhere that makes more sense?
// it being here is mostly just a leftover from an earlier attempt but works fine so why bother >w<
Player* pTargetPlayerForLoadGameTexture = nullptr;
auto LoadGameTexture = (DevTexture*(__stdcall*)(int, const char*, int, int))0x54D8D0;
DevTexture* __stdcall LoadGameTextureHooked(int a1, const char *src, int a3, int a4) {
	// just return the original function if this player is already in the list
	for (auto& player : aPlayers) {
		if (player == pTargetPlayerForLoadGameTexture) return LoadGameTexture(a1, src, a3, a4);
	}
	aPlayers.push_back(pTargetPlayerForLoadGameTexture);
	return LoadGameTexture(a1, src, a3, a4);
}

uintptr_t LoadSkinTextureASM_jmp = 0x431005;
void __attribute__((naked)) LoadSkinTextureASM() {
	__asm__ (
		"mov eax, [esp+0x5C]\n\t"
		"mov eax, [eax+0x463C]\n\t"
		"mov %2, eax\n\t"
		"call %0\n\t"
		"mov [ebp+0], eax\n\t"
		"jmp %1\n\t"
			:
			:  "i" (LoadGameTextureHooked), "m" (LoadSkinTextureASM_jmp), "m" (pTargetPlayerForLoadGameTexture)
	);
}

IDirect3DTexture9* TryLoadCustomTexture(std::string path) {
	// load tga first, game initially has tga in the path
	if (auto tex = LoadTexture(path.c_str())) return tex;

	// then load dds if tga doesn't exist
	for (int i = 0; i < 3; i++) path.pop_back();
	path += "dds";
	if (auto tex = LoadTexture(path.c_str())) return tex;

	// load png as a last resort
	for (int i = 0; i < 3; i++) path.pop_back();
	path += "png";
	if (auto tex = LoadTexture(path.c_str())) return tex;
	return nullptr;
}

void ReplaceTextureWithCustom(DevTexture* pTexture) {
	if (auto texture = TryLoadCustomTexture(pTexture->sPath)) {
		if (pTexture->pD3DTexture) pTexture->pD3DTexture->Release();
		pTexture->pD3DTexture = texture;
		WriteLog("Replaced texture " + (std::string)pTexture->sPath + " with loose files");
	}
}

void SetCarTexturesToCustom(Car* car) {
	if (!car) return;

	int i = 0;
	while (&car->pTextureNodes[i] != car->pTextureNodesEnd) {
		if (auto node = car->pTextureNodes[i]) {
			if (auto texture = node->m_pTexture) {
				ReplaceTextureWithCustom(texture);
			}
		}
		i++;
	}

	ReplaceTextureWithCustom(car->pSkinDamaged);
	ReplaceTextureWithCustom(car->pLightsDamaged);
	ReplaceTextureWithCustom(car->pLightsGlow);
	ReplaceTextureWithCustom(car->pLightsGlowLit);
}

bool bKeyPressed;
bool bKeyPressedLast;
bool IsKeyJustPressed(int key) {
	if (key <= 0) return false;
	if (key >= 255) return false;

	bKeyPressedLast = bKeyPressed;
	bKeyPressed = (GetAsyncKeyState(key) & 0x8000) != 0;
	return bKeyPressed && !bKeyPressedLast;
}

auto RenderRace = (void(__stdcall*)(void*, int))0x479200;
void __stdcall RenderRaceHooked(void* a1, int a2) {
	if (pLocalPlayer && !aPlayers.empty() && (bFirstFrameToReplaceTextures || IsKeyJustPressed(nRefreshKey))) {
		if (bReplaceAllCarsTextures) {
			for (auto& player : aPlayers) {
				SetCarTexturesToCustom(player->pCar);
			}
		}
		else SetCarTexturesToCustom(pLocalPlayer->pCar);
		bFirstFrameToReplaceTextures = false;
	}
	RenderRace(a1, a2);
}

BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH: {
			if (NyaHookLib::GetEntryPoint() != 0x202638) {
				MessageBoxA(nullptr, "Unsupported game version! Make sure you're using v1.2 (.exe size of 2990080 bytes)", "nya?!~", MB_ICONERROR);
				return TRUE;
			}

			auto config = toml::parse_file("FlatOut2SkinChanger_gcp.toml");
			bReplaceAllCarsTextures = config["main"]["replace_all_cars"].value_or(false);
			nRefreshKey = config["main"]["reload_key"].value_or(VK_INSERT);

			NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x45DD25, &CollectLocalPlayerASM);
			LoadGameTexture = (DevTexture*(__stdcall*)(int, const char*, int, int))NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x430FFD, &LoadSkinTextureASM);
			RenderRace = (void(__stdcall*)(void*, int))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x45F781, &RenderRaceHooked);
		} break;
		default:
			break;
	}
	return TRUE;
}