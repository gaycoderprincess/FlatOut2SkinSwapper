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
	DevTexture* m_pTexture;				// 00-04
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