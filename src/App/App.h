
/* * * * * * * * * * * * * Author's note * * * * * * * * * * * *\
*   _       _   _       _   _       _   _       _     _ _ _ _   *
*  |_|     |_| |_|     |_| |_|_   _|_| |_|     |_|  _|_|_|_|_|  *
*  |_|_ _ _|_| |_|     |_| |_|_|_|_|_| |_|     |_| |_|_ _ _     *
*  |_|_|_|_|_| |_|     |_| |_| |_| |_| |_|     |_|   |_|_|_|_   *
*  |_|     |_| |_|_ _ _|_| |_|     |_| |_|_ _ _|_|  _ _ _ _|_|  *
*  |_|     |_|   |_|_|_|   |_|     |_|   |_|_|_|   |_|_|_|_|    *
*                                                               *
*                     http://www.humus.name                     *
*                                                                *
* This file is a part of the work done by Humus. You are free to   *
* use the code in any way you like, modified, unmodified or copied   *
* into your own work. However, I expect you to respect these points:  *
*  - If you use this file and its contents unmodified, or use a major *
*    part of this file, please credit the author and leave this note. *
*  - For use in anything commercial, please request my approval.     *
*  - Share your work and ideas too as much as you can.             *
*                                                                *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "../Framework3/Direct3D10/D3D10App.h"
#include "../Framework3/Util/Model.h"
#include "../Framework3/Util/BSP.h"

struct Light
{
	float3 position;
	float radius;
};

const int LIGHT_COUNT = 19;

class App : public D3D10App
{
public:
	char *getTitle() const { return "Geometry Buffer Anti-Aliasing"; }

	void moveCamera(const float3 &dir);
	void resetCamera();

	bool onKey(const uint key, const bool pressed);
    bool onMouseButton(int x, int y, MouseButton button, bool pressed);
    void onSize(const int w, const int h);

	bool init();
	void exit();

	bool initAPI();
	void exitAPI();

	bool load();
	void unload();

	void drawFrame();
    void drawZoomWindow(TextureID sourceRT);

protected:
	ShaderID m_FillBuffers[2], m_SkyBox, m_Ambient, m_Lighting, m_AntiAlias;
    ShaderID m_Quad, m_ZoomBorder;
	TextureID m_BaseTex[6], m_BumpTex[6], m_SkyBoxTex;
	TextureID m_BaseRT, m_NormalRT, m_DepthRT, m_GeometryRT, m_ResultRT;

	SamplerStateID m_BaseFilter, m_PointClamp;
	BlendStateID m_BlendAdd, m_NoBlend;
	DepthStateID m_DepthTest;

	Light m_Lights[LIGHT_COUNT];

	Model *m_Map;
	Model *m_Sphere;
	BSP m_BSP;

	CheckBox *m_UseGBAA;
    CheckBox *m_TweakGPAA;
	CheckBox *m_RotateSkyBox;

    IndexBufferID m_QuadIB, m_ZoomIB;
    VertexBufferID m_QuadVB, m_ZoomVB;
    VertexFormatID m_QuadVF;
    const float quadZoom = 0.05f;
    float2 m_ZoomOffset;
};
