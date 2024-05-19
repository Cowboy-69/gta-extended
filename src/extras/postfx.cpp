#define WITHD3D
#include "common.h"

#ifdef EXTENDED_COLOURFILTER

#ifndef LIBRW
#error "Need librw for EXTENDED_COLOURFILTER"
#endif

#include "main.h"
#include "RwHelper.h"
#include "Camera.h"
#include "MBlur.h"
#include "postfx.h"

RwRaster *CPostFX::pFrontBuffer;
RwRaster *CPostFX::pBackBuffer;
bool CPostFX::bJustInitialised;
int CPostFX::EffectSwitch = POSTFX_NORMAL;
bool CPostFX::BlurOn = false;
bool CPostFX::MotionBlurOn = false;

static RwIm2DVertex Vertex[4];
static RwIm2DVertex Vertex2[4];
static RwImVertexIndex Index[6] = { 0, 1, 2, 0, 2, 3 };

#ifdef RW_D3D9
void *colourfilterVC_PS;
void *contrast_PS;
#ifdef EX_YCBCR_CORRECTION
void *grading_PS;
#endif
#endif
#ifdef RW_OPENGL
int32 u_blurcolor;
int32 u_contrastAdd;
int32 u_contrastMult;
rw::gl3::Shader *colourFilterVC;
rw::gl3::Shader *contrast;
#endif

#ifdef EX_YCBCR_CORRECTION
bool CPostFX::m_bYCbCrFilter = false;
float CPostFX::m_lumaScale = 219.0f / 255.0f;
float CPostFX::m_lumaOffset = 16.0f / 255.0f;
float CPostFX::m_cbScale = 1.23f;
float CPostFX::m_cbOffset = 0.0f;
float CPostFX::m_crScale = 1.23f;
float CPostFX::m_crOffset = 0.0f;
#endif

void
CPostFX::InitOnce(void)
{
#ifdef RW_OPENGL
	u_blurcolor = rw::gl3::registerUniform("u_blurcolor");
	u_contrastAdd = rw::gl3::registerUniform("u_contrastAdd");
	u_contrastMult = rw::gl3::registerUniform("u_contrastMult");
#endif
}

void
CPostFX::Open(RwCamera *cam)
{
	if(pFrontBuffer)
		Close();

	uint32 width  = Pow(2.0f, int32(log2(RwRasterGetWidth (RwCameraGetRaster(cam))))+1);
	uint32 height = Pow(2.0f, int32(log2(RwRasterGetHeight(RwCameraGetRaster(cam))))+1);
	uint32 depth  = RwRasterGetDepth(RwCameraGetRaster(cam));
	pFrontBuffer = RwRasterCreate(width, height, depth, rwRASTERTYPECAMERATEXTURE);
	pBackBuffer = RwRasterCreate(width, height, depth, rwRASTERTYPECAMERATEXTURE);
	bJustInitialised = true;

	float zero, xmax, ymax;

	if(RwRasterGetDepth(RwCameraGetRaster(cam)) == 16){
		zero = HALFPX;
		xmax = width + HALFPX;
		ymax = height + HALFPX;
	}else{
		zero = -HALFPX;
		xmax = width - HALFPX;
		ymax = height - HALFPX;
	}

	RwIm2DVertexSetScreenX(&Vertex[0], zero);
	RwIm2DVertexSetScreenY(&Vertex[0], zero);
	RwIm2DVertexSetScreenZ(&Vertex[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[0], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[0], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[0], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[0], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[0], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex[1], zero);
	RwIm2DVertexSetScreenY(&Vertex[1], ymax);
	RwIm2DVertexSetScreenZ(&Vertex[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[1], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[1], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[1], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[1], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[1], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex[2], xmax);
	RwIm2DVertexSetScreenY(&Vertex[2], ymax);
	RwIm2DVertexSetScreenZ(&Vertex[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[2], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[2], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[2], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[2], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[2], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex[3], xmax);
	RwIm2DVertexSetScreenY(&Vertex[3], zero);
	RwIm2DVertexSetScreenZ(&Vertex[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex[3], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex[3], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex[3], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex[3], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex[3], 255, 255, 255, 255);


	RwIm2DVertexSetScreenX(&Vertex2[0], zero + 2.0f);
	RwIm2DVertexSetScreenY(&Vertex2[0], zero + 2.0f);
	RwIm2DVertexSetScreenZ(&Vertex2[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex2[0], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex2[0], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex2[0], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex2[0], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex2[0], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex2[1], 2.0f);
	RwIm2DVertexSetScreenY(&Vertex2[1], ymax + 2.0f);
	RwIm2DVertexSetScreenZ(&Vertex2[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex2[1], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex2[1], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex2[1], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex2[1], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex2[1], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex2[2], xmax + 2.0f);
	RwIm2DVertexSetScreenY(&Vertex2[2], ymax + 2.0f);
	RwIm2DVertexSetScreenZ(&Vertex2[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex2[2], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex2[2], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex2[2], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex2[2], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex2[2], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX(&Vertex2[3], xmax + 2.0f);
	RwIm2DVertexSetScreenY(&Vertex2[3], zero + 2.0f);
	RwIm2DVertexSetScreenZ(&Vertex2[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetCameraZ(&Vertex2[3], RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetRecipCameraZ(&Vertex2[3], 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetU(&Vertex2[3], 1.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetV(&Vertex2[3], 0.0f, 1.0f/RwCameraGetNearClipPlane(cam));
	RwIm2DVertexSetIntRGBA(&Vertex2[3], 255, 255, 255, 255);


#ifdef RW_D3D9
#include "shaders/obj/colourfilterVC_PS.inc"
	colourfilterVC_PS = rw::d3d::createPixelShader(colourfilterVC_PS_cso);
#include "shaders/obj/contrastPS.inc"
	contrast_PS = rw::d3d::createPixelShader(contrastPS_cso);
#ifdef EX_YCBCR_CORRECTION
#include "shaders/obj/gradingPS.inc"
	grading_PS = rw::d3d::createPixelShader(gradingPS_cso);
#endif
#endif
#ifdef RW_OPENGL
	using namespace rw::gl3;

	{
#include "shaders/obj/im2d_vert.inc"
#include "shaders/obj/colourfilterVC_frag.inc"
	const char *vs[] = { shaderDecl, header_vert_src, im2d_vert_src, nil };
	const char *fs[] = { shaderDecl, header_frag_src, colourfilterVC_frag_src, nil };
	colourFilterVC = Shader::create(vs, fs);
	assert(colourFilterVC);
	}

	{
#include "shaders/obj/im2d_vert.inc"
#include "shaders/obj/contrast_frag.inc"
	const char *vs[] = { shaderDecl, header_vert_src, im2d_vert_src, nil };
	const char *fs[] = { shaderDecl, header_frag_src, contrast_frag_src, nil };
	contrast = Shader::create(vs, fs);
	assert(contrast);
	}
#ifdef EX_YCBCR_CORRECTION
	// TODO
#endif

#endif
}

void
CPostFX::Close(void)
{
	if(pFrontBuffer){
		RwRasterDestroy(pFrontBuffer);
		pFrontBuffer = nil;
	}
	if(pBackBuffer){
		RwRasterDestroy(pBackBuffer);
		pBackBuffer = nil;
	}
#ifdef RW_D3D9
	if(colourfilterVC_PS){
		rw::d3d::destroyPixelShader(colourfilterVC_PS);
		colourfilterVC_PS = nil;
	}
	if(contrast_PS){
		rw::d3d::destroyPixelShader(contrast_PS);
		contrast_PS = nil;
	}
#ifdef EX_YCBCR_CORRECTION
	if (grading_PS) {
		rw::d3d::destroyPixelShader(grading_PS);
		grading_PS = nil;
	}
#endif
#endif
#ifdef RW_OPENGL
	if(colourFilterVC){
		colourFilterVC->destroy();
		colourFilterVC = nil;
	}
	if(contrast){
		contrast->destroy();
		contrast = nil;
	}
#ifdef EX_YCBCR_CORRECTION
	// TODO
#endif
#endif
}

void
CPostFX::RenderOverlayBlur(RwCamera *cam, int32 r, int32 g, int32 b, int32 a)
{
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, pFrontBuffer);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

	RwIm2DVertexSetIntRGBA(&Vertex[0], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex[1], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex[2], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex[3], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex2[0], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex2[1], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex2[2], r*2, g*2, b*2, 30);
	RwIm2DVertexSetIntRGBA(&Vertex2[3], r*2, g*2, b*2, 30);

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, BlurOn ? Vertex2 : Vertex, 4, Index, 6);


	RwIm2DVertexSetIntRGBA(&Vertex2[0], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[0], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex2[1], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[1], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex2[2], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[2], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex2[3], r, g, b, a);
	RwIm2DVertexSetIntRGBA(&Vertex[3], r, g, b, a);

	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDONE);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDONE);

	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, BlurOn ? Vertex2 : Vertex, 4, Index, 6);
}

void
CPostFX::RenderOverlaySniper(RwCamera *cam, int32 r, int32 g, int32 b, int32 a)
{
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, pFrontBuffer);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);

	RwIm2DVertexSetIntRGBA(&Vertex[0], r, g, b, 80);
	RwIm2DVertexSetIntRGBA(&Vertex[1], r, g, b, 80);
	RwIm2DVertexSetIntRGBA(&Vertex[2], r, g, b, 80);
	RwIm2DVertexSetIntRGBA(&Vertex[3], r, g, b, 80);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);
}

float CPostFX::Intensity = 1.0f;

void
CPostFX::RenderOverlayShader(RwCamera *cam, int32 r, int32 g, int32 b, int32 a)
{
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, pBackBuffer);

	if(EffectSwitch == POSTFX_MOBILE){
		float mult[3], add[3];
		mult[0] = (r-64)/256.0f + 1.4f;
		mult[1] = (g-64)/256.0f + 1.4f;
		mult[2] = (b-64)/256.0f + 1.4f;
		add[0] = r/1536.f - 0.05f;
		add[1] = g/1536.f - 0.05f;
		add[2] = b/1536.f - 0.05f;
#ifdef RW_D3D9
		rw::d3d::d3ddevice->SetPixelShaderConstantF(10, mult, 1);
		rw::d3d::d3ddevice->SetPixelShaderConstantF(11, add, 1);

		rw::d3d::im2dOverridePS = contrast_PS;
#endif
#ifdef RW_OPENGL
		rw::gl3::im2dOverrideShader = contrast;
		contrast->use();
		glUniform3fv(contrast->uniformLocations[u_contrastMult], 1, mult);
		glUniform3fv(contrast->uniformLocations[u_contrastAdd], 1, add);
#endif
	}else{
		float f = Intensity;
		float blurcolors[4];
		blurcolors[0] = r*f/255.0f;
		blurcolors[1] = g*f/255.0f;
		blurcolors[2] = b*f/255.0f;
		blurcolors[3] = 30/255.0f;
#ifdef RW_D3D9
		rw::d3d::d3ddevice->SetPixelShaderConstantF(10, blurcolors, 1);
		rw::d3d::im2dOverridePS = colourfilterVC_PS;
#endif
#ifdef RW_OPENGL
		rw::gl3::im2dOverrideShader = colourFilterVC;
		colourFilterVC->use();
		glUniform4fv(colourFilterVC->uniformLocations[u_blurcolor], 1, blurcolors);
#endif
	}
	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);
#ifdef RW_D3D9
	rw::d3d::im2dOverridePS = nil;
#endif
#ifdef RW_OPENGL
	rw::gl3::im2dOverrideShader = nil;
#endif
}

void
CPostFX::RenderMotionBlur(RwCamera *cam, uint32 blur)
{
	if(blur == 0)
		return;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, pFrontBuffer);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	RwIm2DVertexSetIntRGBA(&Vertex[0], 255, 255, 255, blur);
	RwIm2DVertexSetIntRGBA(&Vertex[1], 255, 255, 255, blur);
	RwIm2DVertexSetIntRGBA(&Vertex[2], 255, 255, 255, blur);
	RwIm2DVertexSetIntRGBA(&Vertex[3], 255, 255, 255, blur);

	RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);
}

bool
CPostFX::NeedBackBuffer(void)
{
	// Current frame -- needed for non-blur effect
	switch(EffectSwitch){
	case POSTFX_OFF:
	case POSTFX_SIMPLE:
		// no actual rendering here
		return false;
	case POSTFX_NORMAL:
		if(MotionBlurOn)
			return false;
		else
			return true;
	case POSTFX_MOBILE:
		return true;
	}
	return false;
}

bool
CPostFX::NeedFrontBuffer(int32 type)
{
	// Last frame -- needed for motion blur
	if(CMBlur::Drunkness > 0.0f)
		return true;
	if(type == MOTION_BLUR_SNIPER)
		return true;

	switch(EffectSwitch){
	case POSTFX_OFF:
	case POSTFX_SIMPLE:
		// no actual rendering here
		return false;
	case POSTFX_NORMAL:
		if(MotionBlurOn)
			return true;
		else
			return false;
	case POSTFX_MOBILE:
		return false;
	}
	return false;
}

void
CPostFX::GetBackBuffer(RwCamera *cam)
{
	RwRasterPushContext(pBackBuffer);
	RwRasterRenderFast(RwCameraGetRaster(cam), 0, 0);
	RwRasterPopContext();
}

#ifdef EX_YCBCR_CORRECTION
static RwMatrix RGB2YUV = {
	{  0.299f,	-0.168736f,	 0.500f }, 0,
	{  0.587f,	-0.331264f,	-0.418688f }, 0,
	{  0.114f,	 0.500f,	-0.081312f }, 0,
	{  0.000f,	 0.000f,	 0.000f }, 0,
};

static RwMatrix YUV2RGB = {
	{  1.000f,	 1.000f,	 1.000f }, 0,
	{  0.000f,	-0.344136f,	 1.772f }, 0,
	{  1.402f,	-0.714136f,	 0.000f }, 0,
	{  0.000f,	 0.000f,	 0.000f }, 0,
};

struct Grade
{
	rw::float32 r, g, b, a;
};
#endif

void
CPostFX::Render(RwCamera *cam, uint32 red, uint32 green, uint32 blue, uint32 blur, int32 type, uint32 bluralpha)
{
	PUSH_RENDERGROUP("CPostFX::Render");

	if(pFrontBuffer == nil)
		Open(cam);
	assert(pFrontBuffer);
	assert(pBackBuffer);

	if(type == MOTION_BLUR_LIGHT_SCENE){
		SmoothColor(red, green, blue, blur);
		red = AvgRed;
		green = AvgGreen;
		blue = AvgBlue;
		blur = AvgAlpha;
	}

	if(NeedBackBuffer())
		GetBackBuffer(cam);

	DefinedState();

	RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);

	if(type == MOTION_BLUR_SNIPER){
		if(!bJustInitialised)
			RenderOverlaySniper(cam, red, green, blue, blur);
	}else switch(EffectSwitch){
	case POSTFX_OFF:
	case POSTFX_SIMPLE:
		// no actual rendering here
		break;
	case POSTFX_NORMAL:
		if(MotionBlurOn){
			if(!bJustInitialised)
				RenderOverlayBlur(cam, red, green, blue, blur);
		}else{
			RenderOverlayShader(cam, red, green, blue, blur);
		}
		break;
	case POSTFX_MOBILE:
		RenderOverlayShader(cam, red, green, blue, blur);
		break;
	}

	if(!bJustInitialised)
		RenderMotionBlur(cam, 175.0f * CMBlur::Drunkness);

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);

	if(NeedFrontBuffer(type)){
		RwRasterPushContext(pFrontBuffer);
		RwRasterRenderFast(RwCameraGetRaster(cam), 0, 0);
		RwRasterPopContext();
		bJustInitialised = false;
	}else
		bJustInitialised = true;

	POP_RENDERGROUP();

#ifdef EX_YCBCR_CORRECTION // Render
	if (m_bYCbCrFilter) {
		RwRasterPushContext(pFrontBuffer);
		RwRasterRenderFast(RwCameraGetRaster(cam), 0, 0);
		RwRasterPopContext();

		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
		RwRenderStateSet(rwRENDERSTATEFOGENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)pFrontBuffer);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

		RwMatrix m = RGB2YUV;

		RwMatrix m2;
		m2.right.x = m_lumaScale;
		m2.up.x = 0.0f;
		m2.at.x = 0.0f;
		m2.pos.x = m_lumaOffset;
		m2.right.y = 0.0f;
		m2.up.y = m_cbScale;
		m2.at.y = 0.0f;
		m2.pos.y = m_cbOffset;
		m2.right.z = 0.0f;
		m2.up.z = 0.0f;
		m2.at.z = m_crScale;
		m2.pos.z = m_crOffset;

		RwMatrixOptimize(&m2, nil);

		RwMatrixTransform(&m, &m2, rwCOMBINEPOSTCONCAT);
		RwMatrixTransform(&m, &YUV2RGB, rwCOMBINEPOSTCONCAT);
		Grade red2, green2, blue2;
		red2.r = m.right.x;
		red2.g = m.up.x;
		red2.b = m.at.x;
		red2.a = m.pos.x;
		green2.r = m.right.y;
		green2.g = m.up.y;
		green2.b = m.at.y;
		green2.a = m.pos.y;
		blue2.r = m.right.z;
		blue2.g = m.up.z;
		blue2.b = m.at.z;
		blue2.a = m.pos.z;

		rw::d3d::d3ddevice->SetPixelShaderConstantF(0, (float*)&red2, 1);
		rw::d3d::d3ddevice->SetPixelShaderConstantF(1, (float*)&green2, 1);
		rw::d3d::d3ddevice->SetPixelShaderConstantF(2, (float*)&blue2, 1);

		rw::d3d::im2dOverridePS = grading_PS;
		RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST, Vertex, 4, Index, 6);
		rw::d3d::im2dOverridePS = nil;

		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)NULL);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	}
#endif
}

int CPostFX::PrevRed[NUMAVERAGE], CPostFX::AvgRed;
int CPostFX::PrevGreen[NUMAVERAGE], CPostFX::AvgGreen;
int CPostFX::PrevBlue[NUMAVERAGE], CPostFX::AvgBlue;
int CPostFX::PrevAlpha[NUMAVERAGE], CPostFX::AvgAlpha;
int CPostFX::Next;
int CPostFX::NumValues;

// This is rather annoying...the blur color can flicker slightly
// which becomes very visible when amplified by the shader
void
CPostFX::SmoothColor(uint32 red, uint32 green, uint32 blue, uint32 alpha)
{
	PrevRed[Next] = red;
	PrevGreen[Next] = green;
	PrevBlue[Next] = blue;
	PrevAlpha[Next] = alpha;
	Next = (Next+1) % NUMAVERAGE;
	NumValues = Min(NumValues+1, NUMAVERAGE);

	AvgRed = 0;
	AvgGreen = 0;
	AvgBlue = 0;
	AvgAlpha = 0;
	for(int i = 0; i < NumValues; i++){
		AvgRed += PrevRed[i];
		AvgGreen += PrevGreen[i];
		AvgBlue += PrevBlue[i];
		AvgAlpha += PrevAlpha[i];
	}
	AvgRed /= NumValues;
	AvgGreen /= NumValues;
	AvgBlue /= NumValues;
	AvgAlpha /= NumValues;
}

#endif
