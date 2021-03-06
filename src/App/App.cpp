
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

#include "App.h"

BaseApp *app = new App();

void App::moveCamera(const float3 &dir)
{
	float3 newPos = camPos + dir * (speed * frameTime);

	float3 point;
	const BTri *tri;
	if (m_BSP.intersects(camPos, newPos, &point, &tri))
	{
		newPos = point + tri->plane.xyz();
	}
	m_BSP.pushSphere(newPos, 35);

	camPos = newPos;
}

void App::resetCamera()
{
	//camPos = vec3(-853, -101, -965);
	//wx = 0.153f;
	//wy = -0.655f;
    camPos = vec3(-286.495880f, -201.800903f, 988.192078f);
    wx = 1.48800111f;
    wy = -0.210998490f;
}

void App::updateZoomOffset(int x, int y)
{
    m_ZoomOffset.x = float(x) / getWidth();
    m_ZoomOffset.y = float(y) / getHeight();
}

bool App::onKey(const uint key, const bool pressed)
{
	if (D3D10App::onKey(key, pressed)) return true;

	if (pressed)
	{
		if (key == KEY_F5)
		{
			m_UseGBAA->setChecked(!m_UseGBAA->isChecked());
		}
	}

	return false;
}

bool App::onMouseButton(int x, int y, MouseButton button, bool pressed)
{
    bool result = D3D10App::onMouseButton(x, y, button, pressed);
    if (!mouseCaptured && button == MOUSE_RIGHT)
    {
        m_ZoomTracking = pressed;
        if (pressed)
            updateZoomOffset(x, y);
        return true;
    }
    return result;
}

bool App::onMouseMove(int x, int y, int dx, int dy)
{
    if (!mouseCaptured && m_ZoomTracking)
        updateZoomOffset(x, y);
    return D3D10App::onMouseMove(x, y, dx, dy);
}

void App::onSize(const int w, const int h)
{
	D3D10App::onSize(w, h);

	if (renderer)
	{
		// Make sure render targets are the size of the window
		renderer->resizeRenderTarget(m_BaseRT,     w, h, 1, 1, 1);
		renderer->resizeRenderTarget(m_NormalRT,   w, h, 1, 1, 1);
		renderer->resizeRenderTarget(m_DepthRT,    w, h, 1, 1, 1);
		renderer->resizeRenderTarget(m_ResultRT,   w, h, 1, 1, 1);
		renderer->resizeRenderTarget(m_GeometryRT, w, h, 1, 1, 1);
        renderer->resizeRenderTarget(m_InvGeometryRT, w, h, 1, 1, 1);
	}
}

bool App::init()
{
	// No framework-created depth buffer
	depthBits = 0;

	// Load map
	m_Map = new Model();
	if (!m_Map->loadObj("../Models/Corridor3/Map.obj")) return false;
	m_Map->scale(0, float3(1, 1, -1));

	// Create BSP for collision detection
	uint nIndices = m_Map->getIndexCount();
	float3 *src = (float3 *) m_Map->getStream(0).vertices;
	uint *inds = m_Map->getStream(0).indices;

	for (uint i = 0; i < nIndices; i += 3)
	{
		const float3 &v0 = src[inds[i]];
		const float3 &v1 = src[inds[i + 1]];
		const float3 &v2 = src[inds[i + 2]];

		m_BSP.addTriangle(v0, v1, v2);
	}
	m_BSP.build();

	m_Map->computeTangentSpace(true);

// 	m_Map->cleanUp();

	// Create light sphere for deferred shading
	m_Sphere = new Model();
	m_Sphere->createSphere(3);

	// Initialize all lights
	m_Lights[ 0].position = float3( 576, 96,    0); m_Lights[ 0].radius = 640.0f;
	m_Lights[ 1].position = float3( 0,   96,  576); m_Lights[ 1].radius = 640.0f;
	m_Lights[ 2].position = float3(-576, 96,    0); m_Lights[ 2].radius = 640.0f;
	m_Lights[ 3].position = float3( 0,   96, -576); m_Lights[ 3].radius = 640.0f;
	m_Lights[ 4].position = float3(1792, 96,  320); m_Lights[ 4].radius = 550.0f;
	m_Lights[ 5].position = float3(1792, 96, -320); m_Lights[ 5].radius = 550.0f;
	m_Lights[ 6].position = float3(-192, 96, 1792); m_Lights[ 6].radius = 550.0f;
	m_Lights[ 7].position = float3(-832, 96, 1792); m_Lights[ 7].radius = 550.0f;
	m_Lights[ 8].position = float3(1280, 32,  192); m_Lights[ 8].radius = 450.0f;
	m_Lights[ 9].position = float3(1280, 32, -192); m_Lights[ 9].radius = 450.0f;
	m_Lights[10].position = float3(-320, 32, 1280); m_Lights[10].radius = 450.0f;
	m_Lights[11].position = float3(-704, 32, 1280); m_Lights[11].radius = 450.0f;
	m_Lights[12].position = float3( 960, 32,  640); m_Lights[12].radius = 450.0f;
	m_Lights[13].position = float3( 960, 32, -640); m_Lights[13].radius = 450.0f;
	m_Lights[14].position = float3( 640, 32, -960); m_Lights[14].radius = 450.0f;
	m_Lights[15].position = float3(-640, 32, -960); m_Lights[15].radius = 450.0f;
	m_Lights[16].position = float3(-960, 32,  640); m_Lights[16].radius = 450.0f;
	m_Lights[17].position = float3(-960, 32, -640); m_Lights[17].radius = 450.0f;
	m_Lights[18].position = float3( 640, 32,  960); m_Lights[18].radius = 450.0f;

	// Init GUI components
	int tab = configDialog->addTab("GBAA");
	configDialog->addWidget(tab, m_UseGBAA = new CheckBox(0, 0, 150, 36, "Use GBAA", true));
    configDialog->addWidget(tab, m_TweakGPAA = new CheckBox(0, 40, 250, 36, "Use GBAA tweak", true));
    configDialog->addWidget(tab, m_ShowEdges = new CheckBox(0, 80, 250, 36, "Highlight edges", false));
    configDialog->addWidget(tab, m_RotateSkyBox = new CheckBox(0, 120, 200, 36, "Rotate SkyBox", true));

	return true;
}

void App::exit()
{
	delete m_Sphere;
	delete m_Map;
}

bool App::initAPI()
{
	// Override the user's MSAA settings and disable the control
	antiAlias->setEnabled(false);
	return D3D10App::initAPI(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, DXGI_FORMAT_UNKNOWN, 1, NO_SETTING_CHANGE);
}

void App::exitAPI()
{
	D3D10App::exitAPI();
}

bool App::load()
{
	// Shaders
	if ((m_FillBuffers[0] = renderer->addShader("../Shaders/FillBuffers.shd"                        )) == SHADER_NONE) return false;
	if ((m_FillBuffers[1] = renderer->addShader("../Shaders/FillBuffers.shd", "#define ALPHA_TEST\n")) == SHADER_NONE) return false;
	if ((m_SkyBox    = renderer->addShader("../Shaders/SkyBox.shd"  )) == SHADER_NONE) return false;
	if ((m_Ambient   = renderer->addShader("../Shaders/Ambient.shd" )) == SHADER_NONE) return false;
	if ((m_Lighting  = renderer->addShader("../Shaders/Lighting.shd")) == SHADER_NONE) return false;
	if ((m_AntiAlias = renderer->addShader("../Shaders/GBAA.shd"    )) == SHADER_NONE) return false;
    if ((m_ZoomBorder = renderer->addShader("../Shaders/ZoomBorder.shd")) == SHADER_NONE) return false;
    if ((m_Quad = renderer->addShader("../Shaders/Quad.shd")) == SHADER_NONE) return false;

	// Samplerstates
	if ((m_BaseFilter = renderer->addSamplerState(TRILINEAR_ANISO, WRAP, WRAP, WRAP)) == SS_NONE) return false;
	if ((m_PointClamp = renderer->addSamplerState(NEAREST, CLAMP, CLAMP, CLAMP)) == SS_NONE) return false;

	// Main render targets
	if ((m_BaseRT     = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGBA8,  1, SS_NONE, SRGB)) == TEXTURE_NONE) return false;
	if ((m_NormalRT   = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGBA8S, 1, SS_NONE)) == TEXTURE_NONE) return false;
	if ((m_DepthRT    = renderer->addRenderDepth (width, height, 1,       FORMAT_D16,    1, SS_NONE, SAMPLE_DEPTH)) == TEXTURE_NONE) return false;
	if ((m_ResultRT   = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RGBA8,  1, SS_NONE, SRGB)) == TEXTURE_NONE) return false;
	if ((m_GeometryRT = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_RG16F,  1, SS_NONE)) == TEXTURE_NONE) return false;
    if ((m_InvGeometryRT = renderer->addRenderTarget(width, height, 1, 1, 1, FORMAT_R16F,  1, SS_NONE)) == TEXTURE_NONE) return false;

	// Textures
	if ((m_BaseTex[0] = renderer->addTexture  ("../Textures/floor_wood_4.dds",                      true, m_BaseFilter, SRGB)) == TEXTURE_NONE) return false;
	if ((m_BumpTex[0] = renderer->addNormalMap("../Textures/floor_wood_4Bump.dds",   FORMAT_RGBA8S, true, m_BaseFilter      )) == TEXTURE_NONE) return false;

	if ((m_BaseTex[1] = renderer->addTexture  ("../Textures/wood.dds",                              true, m_BaseFilter, SRGB)) == TEXTURE_NONE) return false;
	if ((m_BumpTex[1] = renderer->addNormalMap("../Textures/woodBump.dds",           FORMAT_RGBA8S, true, m_BaseFilter      )) == TEXTURE_NONE) return false;

	if ((m_BaseTex[2] = renderer->addTexture  ("../Textures/floor_wood_3.dds",                      true, m_BaseFilter, SRGB)) == TEXTURE_NONE) return false;
	if ((m_BumpTex[2] = renderer->addNormalMap("../Textures/floor_wood_3Bump.dds",   FORMAT_RGBA8S, true, m_BaseFilter      )) == TEXTURE_NONE) return false;

	if ((m_BaseTex[3] = renderer->addTexture  ("../Textures/Tx_imp_wall_01_small.dds",              true, m_BaseFilter, SRGB)) == TEXTURE_NONE) return false;
	if ((m_BumpTex[3] = renderer->addNormalMap("../Textures/Tx_imp_wall_01Bump.dds", FORMAT_RGBA8S, true, m_BaseFilter      )) == TEXTURE_NONE) return false;

	if ((m_BaseTex[4] = renderer->addTexture  ("../Textures/light2.dds",                            true, m_BaseFilter, SRGB)) == TEXTURE_NONE) return false;
	if ((m_BumpTex[4] = renderer->addNormalMap("../Textures/light2Bump.dds",         FORMAT_RGBA8S, true, m_BaseFilter      )) == TEXTURE_NONE) return false;

	if ((m_BaseTex[5] = renderer->addTexture  ("../Textures/Brown_Hexagons3.dds",                   true, m_BaseFilter, SRGB)) == TEXTURE_NONE) return false;
	if ((m_BumpTex[5] = renderer->addNormalMap("../Textures/Brown_HexagonsBump.dds", FORMAT_RGBA8S, true, m_BaseFilter      )) == TEXTURE_NONE) return false;

	const char *files[] =
	{
		"../Textures/CubeMaps/Drake-equation/posx.jpg", "../Textures/CubeMaps/Drake-equation/negx.jpg",
		"../Textures/CubeMaps/Drake-equation/posy.jpg", "../Textures/CubeMaps/Drake-equation/negy.jpg",
		"../Textures/CubeMaps/Drake-equation/posz.jpg", "../Textures/CubeMaps/Drake-equation/negz.jpg",
	};
	if ((m_SkyBoxTex = renderer->addCubemap(files, true, m_BaseFilter, 1, SRGB)) == TEXTURE_NONE) return false;

	// Blendstates
	if ((m_BlendAdd = renderer->addBlendState(ONE, ONE)) == BS_NONE) return false;
    if ((m_NoBlend = renderer->addBlendState(ONE, ZERO)) == BS_NONE) return false;

	// Depth states - use reversed depth (1 to 0) to improve precision
	if ((m_DepthTest = renderer->addDepthState(true, true, GEQUAL)) == DS_NONE) return false;

	// Upload map to vertex/index buffer
	if (!m_Map->makeDrawable(renderer, true, m_FillBuffers[0])) return false;
	if (!m_Sphere->makeDrawable(renderer, true, m_Lighting)) return false;

    // Create index/vertex buffers for zoom window
    struct QuadVertex
    {
        float3 Position;
        float2 TexCoord;
    };
    // quadZoom = 0.05f
    float2 texCoord[] = 
    {
         {0.0f, 0.0f},
         {0.0f, quadZoom},
         {quadZoom, quadZoom},
         {quadZoom, 0.0f}
    };
    QuadVertex zoomVerts[] = // zero-centered quad
    {
        {{-quadZoom, -quadZoom, 0.0f}, texCoord[0]},
        {{-quadZoom, +quadZoom, 0.0f}, texCoord[1]},
        {{+quadZoom, +quadZoom, 0.0f}, texCoord[2]},
        {{+quadZoom, -quadZoom, 0.0f}, texCoord[3]}
    };
    const float xPos = +0.5f;
    const float yPos = -0.5f;
    const float xSize = 0.5f;
    const float ySize = 0.5f;
    QuadVertex quadVerts[] =
    {
        {{xPos, yPos,                   0.0f}, texCoord[0]},
        {{xPos, yPos - ySize,           0.0f}, texCoord[1]},
        {{xPos + xSize, yPos - ySize,   0.0f}, texCoord[2]},
        {{xPos + xSize, yPos,           0.0f}, texCoord[3]}
    };
    uint16 quadIndices[] = {0, 1, 3, 3, 1, 2};
    if ((m_QuadIB = renderer->addIndexBuffer(elementsOf(quadIndices), sizeof(uint16),
        STATIC, quadIndices)) == IB_NONE)
    {
        return false;
    }
    if ((m_QuadVB = renderer->addVertexBuffer(sizeof(quadVerts), STATIC, quadVerts)) == VB_NONE) return false;
    FormatDesc quadFormat[] =
    {
        {0, TYPE_VERTEX, FORMAT_FLOAT, 3},
        {0, TYPE_TEXCOORD, FORMAT_FLOAT, 2}
    };
    if ((m_QuadVF = renderer->addVertexFormat(quadFormat, elementsOf(quadFormat), m_Quad)) == VF_NONE) return false;
    if ((m_ZoomVB = renderer->addVertexBuffer(sizeof(zoomVerts), STATIC, zoomVerts)) == VB_NONE) return false;
    uint16 zoomIndices[] = {0, 1, 2, 3, 0};
    if ((m_ZoomIB = renderer->addIndexBuffer(elementsOf(zoomIndices), sizeof(uint16),
        STATIC, zoomIndices)) == IB_NONE)
    {
        return false;
    }
    m_ZoomOffset = float2(0.5f, 0.5f);
	return true;
}

void App::unload()
{

}

void App::drawFrame()
{
	const float near_plane = 20.0f;
	const float far_plane = 4000.0f;

	// Reversed depth
	float4x4 projection = toD3DProjection(perspectiveMatrixY(1.2f, width, height, far_plane, near_plane));
	float4x4 view = rotateXY(-wx, -wy);
	view.translate(-camPos);
	float4x4 viewProj = projection * view;
	// Pre-scale-bias the matrix so we can use the screen position directly
	float4x4 viewProjInv = (!viewProj) * (translate(-1.0f, 1.0f, 0.0f) * scale(2.0f, -2.0f, 1.0f));

	Direct3D10Renderer *d3d10_renderer = (Direct3D10Renderer *) renderer;

	TextureID bufferRTs[] = { m_BaseRT, m_NormalRT, m_GeometryRT, m_InvGeometryRT };
	renderer->changeRenderTargets(bufferRTs, elementsOf(bufferRTs), m_DepthRT);
	{
		// Clear to 0.5f, indicating that there is no edge cutting through this pixel.
		d3d10_renderer->clearRenderTarget(m_GeometryRT, float4(0.5f, 0.5f, 0.5f, 0.5f));
        d3d10_renderer->clearRenderTarget(m_InvGeometryRT, float4(0));
		d3d10_renderer->clearDepthTarget(m_DepthRT, 0.0f);

		/*
			Main scene pass.
			This is where the buffers are filled for the later deferred passes.
		*/
		float4 scale_bias(0.5f * width, -0.5f * height, 0.5f * width, 0.5f * height);

		// Opaque surfaces
		renderer->reset();
		renderer->setRasterizerState(cullBack);
		renderer->setDepthState(m_DepthTest);
		renderer->setShader(m_FillBuffers[0]);
		renderer->setShaderConstant4x4f("ViewProj", viewProj);
		renderer->setShaderConstant4f("ScaleBias", scale_bias);
		renderer->setSamplerState("Filter", m_BaseFilter);
		renderer->apply();

		for (uint i = 0; i < 5; i++)
		{
			renderer->setTexture("Base", m_BaseTex[i]);
			renderer->setTexture("Bump", m_BumpTex[i]);
			renderer->applyTextures();

			m_Map->drawBatch(renderer, i);
		}

		// Alpha-tested surfaces
		renderer->reset();
		renderer->setRasterizerState(cullBack);
		renderer->setDepthState(m_DepthTest);
		renderer->setShader(m_FillBuffers[1]);
		renderer->setShaderConstant4x4f("ViewProj", viewProj);
		renderer->setShaderConstant4f("ScaleBias", scale_bias);
		renderer->setTexture("Base", m_BaseTex[5]);
		renderer->setTexture("Bump", m_BumpTex[5]);
		renderer->setSamplerState("Filter", m_BaseFilter);
		renderer->apply();

		m_Map->drawBatch(renderer, 6);
	}


	TextureID target = m_UseGBAA->isChecked()? m_ResultRT : FB_COLOR;

	renderer->changeRenderTarget(target, TEXTURE_NONE);

	{
		/*
			Deferred ambient pass.
		*/
		renderer->reset();
		renderer->setRasterizerState(cullNone);
		renderer->setDepthState(noDepthTest);
		renderer->setShader(m_Ambient);
		renderer->setTexture("Base", m_BaseRT);
		renderer->setSamplerState("Filter", m_PointClamp);
		renderer->apply();

		device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		device->Draw(3, 0);


		/*
			Deferred lighting pass.
		*/
		renderer->reset();
		renderer->setDepthState(noDepthTest);
		renderer->setShader(m_Lighting);
		renderer->setRasterizerState(cullFront);
		renderer->setBlendState(m_BlendAdd);
		renderer->setShaderConstant4x4f("ViewProj", viewProj);
		renderer->setShaderConstant4x4f("ViewProjInv", viewProjInv * scale(1.0f / width, 1.0f / height, 1.0f));
		renderer->setShaderConstant3f("CamPos", camPos);
		renderer->setTexture("Base", m_BaseRT);
		renderer->setTexture("Normal", m_NormalRT);
		renderer->setTexture("Depth", m_DepthRT);
		renderer->apply();

		float2 zw = projection.rows[2].zw();
		for (uint i = 0; i < LIGHT_COUNT; i++)
		{
			float3 lightPos = m_Lights[i].position;
			float radius = m_Lights[i].radius;
			float invRadius = 1.0f / radius;

			// Compute z-bounds
			float4 lPos = view * float4(lightPos, 1.0f);
			float z1 = lPos.z + radius;

			if (z1 > near_plane)
			{
				float z0 = max(lPos.z - radius, near_plane);

				float2 zBounds;
				zBounds.y = saturate(zw.x + zw.y / z0);
				zBounds.x = saturate(zw.x + zw.y / z1);

				renderer->setShaderConstant3f("LightPos", lightPos);
				renderer->setShaderConstant1f("Radius", radius);
				renderer->setShaderConstant1f("InvRadius", invRadius);
				renderer->setShaderConstant2f("ZBounds", zBounds);
				renderer->applyConstants();

				m_Sphere->draw(renderer);
			}
		}
	}


	renderer->changeRenderTarget(target, m_DepthRT);


	// Skybox
	float angle = m_RotateSkyBox->isChecked()? 0.05f * time - 0.7f : 0.0f;

	renderer->reset();
	renderer->setRasterizerState(cullBack);
	renderer->setDepthState(m_DepthTest);
	renderer->setShader(m_SkyBox);
	renderer->setShaderConstant4x4f("ViewProj", viewProj);
	renderer->setShaderConstant4x4f("Rotation", rotateZ(angle));
	renderer->setShaderConstant3f("CamPos", camPos);
	renderer->setTexture("Sky", m_SkyBoxTex);
	renderer->setSamplerState("Filter", m_BaseFilter);
	renderer->apply();

	m_Map->drawBatch(renderer, 5);



	/*
		Geometry Buffer Anti-Aliasing
	*/
	if (m_UseGBAA->isChecked())
	{
		renderer->changeToMainFramebuffer();

		// GBAA resolve pass
		renderer->reset();
		renderer->setDepthState(noDepthTest);
		renderer->setShader(m_AntiAlias);
		renderer->setRasterizerState(cullNone);
		renderer->setShaderConstant2f("PixelSize", float2(1.0f / width, 1.0f / height));
        renderer->setShaderConstant1i("Tweak", m_TweakGPAA->isChecked() ? 1 : 0);
        renderer->setShaderConstant1i("ShowEdges", m_ShowEdges->isChecked() ? 1 : 0);
		renderer->setTexture("BackBuffer", m_ResultRT);
		renderer->setTexture("GeometryBuffer", m_GeometryRT);
        renderer->setTexture("InvGeometryBuffer", m_InvGeometryRT);
		renderer->setSamplerState("Linear", linearClamp);
		renderer->setSamplerState("Point", m_PointClamp);
		renderer->apply();

		renderer->drawArrays(PRIM_TRIANGLES, 0, 3);
	}
    d3d10_renderer->copyTexture(m_ResultRT, FB_COLOR);
    drawZoomWindow(m_ResultRT); // pass m_GeometryRT to visualize gbuffer
}

void App::drawZoomWindow(TextureID sourceRT)
{
    float4x4 worldViewProj = identity4(); // origin = window center
    
    // render zoom window
    renderer->reset();
    renderer->setDepthState(noDepthTest);
    renderer->setRasterizerState(cullNone);
    renderer->setBlendState(m_NoBlend);
    renderer->setShader(m_Quad);
    renderer->setShaderConstant4x4f("WorldViewProj", worldViewProj);
    renderer->setShaderConstant2f("UVOffset", m_ZoomOffset - quadZoom/2);
    renderer->setSamplerState("PointSampler", m_PointClamp);
    renderer->setTexture("BackBuffer", sourceRT);
    renderer->setVertexFormat(m_QuadVF);
    renderer->setVertexBuffer(0, m_QuadVB);
    renderer->setIndexBuffer(m_QuadIB);
    renderer->apply();
    renderer->drawElements(PRIM_TRIANGLES, 0, 6, 0, 4);

    // border for zoom window
    renderer->reset();
    renderer->setDepthState(noDepthTest);
    renderer->setRasterizerState(cullNone);
    renderer->setBlendState(m_NoBlend);
    renderer->setShader(m_ZoomBorder);
    renderer->setShaderConstant4x4f("WorldViewProj", worldViewProj);
    renderer->setShaderConstant2f("UVOffset", float2(0, 0));
    renderer->setVertexFormat(m_QuadVF);
    renderer->setVertexBuffer(0, m_QuadVB);
    renderer->setIndexBuffer(m_ZoomIB);
    renderer->apply();
    renderer->drawElements(PRIM_LINE_STRIP, 0, 5, 0, 4);

    // border for zoom window origin
    renderer->reset();
    renderer->setDepthState(noDepthTest);
    renderer->setRasterizerState(cullNone);
    renderer->setBlendState(m_NoBlend);
    renderer->setShader(m_ZoomBorder);
    renderer->setShaderConstant4x4f("WorldViewProj", worldViewProj);
    renderer->setShaderConstant2f("UVOffset", m_ZoomOffset * 2.0f - 1.0f);
    renderer->setVertexFormat(m_QuadVF);
    renderer->setVertexBuffer(0, m_ZoomVB);
    renderer->setIndexBuffer(m_ZoomIB);
    renderer->apply();
    renderer->drawElements(PRIM_LINE_STRIP, 0, 5, 0, 4);
}
