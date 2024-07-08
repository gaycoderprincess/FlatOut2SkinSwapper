auto& g_pd3dDevice = *(IDirect3DDevice9**)0x8DA788;

class DevTexture {
public:
	uint8_t _0[0x20];
	char* sPath;
	uint8_t _24[0x2C];
	PDIRECT3DTEXTURE9 pD3DTexture;
};

class tTextureNode {
public:
	DevTexture* pTexture;
};

class Car {
public:
	uint8_t _0[0x1C1C];
	tTextureNode** pTextureNodes;
	tTextureNode** pTextureNodesEnd;
	uint8_t _1C24[0x2970];
	DevTexture* pSkinDamaged;
	DevTexture* pLightsDamaged;
	DevTexture* pLightsGlow;
	DevTexture* pLightsGlowLit;
};

class Player {
public:
	uint8_t _0[0x33C];
	Car* pCar;
};

class MenuCar {
public:
	int nCarId;
	uint8_t _4[0x4];
	uint32_t nModelSize;
	void* pModel;
	uint8_t _10[0x4];
	int nSkinId;
	uint8_t _18[0x4];
	uint32_t nSkinSize;
	void* pSkin;
};

class MenuInterface {
public:
	uint8_t _0[0x5AC];
	MenuCar* pMenuCar;
};

struct tGameMain {
	uint8_t _0[0x9C8];
	MenuInterface* pMenuInterface;
};
auto& pGame = *(tGameMain**)0x8E8410;