#include "Coloramen.h"

#include "AEFX_SuiteHelper.h"
#include <adobesdk/DrawbotSuite.h>

static void
copyConvertStringLiteralIntoUTF16(
	const wchar_t* inputString,
	A_UTF16Char* destination)
{
#ifdef AE_OS_MAC
	int length = wcslen(inputString);
	CFRange	range = { 0, 256 };
	range.length = length;
	CFStringRef inputStringCFSR = CFStringCreateWithBytes(kCFAllocatorDefault,
		reinterpret_cast<const UInt8*>(inputString),
		length * sizeof(wchar_t),
		kCFStringEncodingUTF32LE,
		false);
	CFStringGetBytes(inputStringCFSR,
		range,
		kCFStringEncodingUTF16,
		0,
		false,
		reinterpret_cast<UInt8*>(destination),
		length * (sizeof(A_UTF16Char)),
		NULL);
	destination[length] = 0; // Set NULL-terminator, since CFString calls don't set it
	CFRelease(inputStringCFSR);
#elif defined AE_OS_WIN
	size_t length = wcslen(inputString);
	wcscpy_s(reinterpret_cast<wchar_t*>(destination), length + 1, inputString);
#endif
}

static PF_Err
DrawEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	PF_Err					err = PF_Err_NONE, err2 = PF_Err_NONE;

	DRAWBOT_DrawRef			drawing_ref = NULL;
	DRAWBOT_SurfaceRef		surface_ref = NULL;
	DRAWBOT_SupplierRef		supplier_ref = NULL;

	DRAWBOT_Suites* drawbotSuites = AEFX_DrawbotSuitesScoper(in_data, out_data).Get();

	PF_EffectCustomUISuite1* effectCustomUISuiteP;

	ERR(AEFX_AcquireSuite(in_data,
		out_data,
		kPFEffectCustomUISuite,
		kPFEffectCustomUISuiteVersion1,
		NULL,
		(void**)&effectCustomUISuiteP));

	if (!err && effectCustomUISuiteP) {
		// Get the drawing reference by passing context to this new api
		ERR((*effectCustomUISuiteP->PF_GetDrawingReference)(event_extra->contextH, &drawing_ref));

		AEFX_ReleaseSuite(in_data, out_data, kPFEffectCustomUISuite, kPFEffectCustomUISuiteVersion1, NULL);
	}

	// Get the Drawbot supplier from drawing reference; it shouldn't be released like pen or brush (see below)
	ERR(drawbotSuites->drawbot_suiteP->GetSupplier(drawing_ref, &supplier_ref));

	// Get the Drawbot surface from drawing reference; it shouldn't be released like pen or brush (see below)
	ERR(drawbotSuites->drawbot_suiteP->GetSurface(drawing_ref, &surface_ref));

	//DRAWBOT_ColorRGBA normal_color = { 20.0f / PF_MAX_CHAN8, 20.0f / PF_MAX_CHAN8, 20.0f / PF_MAX_CHAN8, 1.0 };
	//DRAWBOT_ColorRGBA white = { 255.0f / PF_MAX_CHAN8, 255.0f / PF_MAX_CHAN8, 255.0f / PF_MAX_CHAN8, 1.0 };
	
	//DRAWBOT_BrushP normal_brush(drawbotSuites->supplier_suiteP, supplier_ref, &normal_color);
	//DRAWBOT_BrushP white_brush(drawbotSuites->supplier_suiteP, supplier_ref, &white);

	// Get the default font size.
	//float default_font_sizeF = 0.0;
	// ERR(drawbotSuites->supplier_suiteP->GetDefaultFontSize(supplier_ref, &default_font_sizeF));
	// Create default font with default size.  Note that you can provide a different font size.
	// DRAWBOT_FontP font_ref(drawbotSuites->supplier_suiteP, supplier_ref, default_font_sizeF);
	//DRAWBOT_FontP font_ref(drawbotSuites->supplier_suiteP, supplier_ref, 40.0);

	if (PF_EA_CONTROL == event_extra->effect_win.area) {
		//DRAWBOT_PathP btn_path_ref(drawbotSuites->supplier_suiteP, supplier_ref);
		//float btn_X = 0.0f, btn_Y = 0.0f, btn_W = 24.0f, btn_H = 24.0f;
		//DRAWBOT_RectF32	btn{
		//	static_cast<float>(event_extra->effect_win.current_frame.left + 0.5 + btn_X),	// Center of the pixel in new drawing model is (0.5, 0.5)
		//	static_cast<float>(event_extra->effect_win.current_frame.top + 0.5 + btn_Y),
		//	btn_W,
		//	btn_H
		//};
		//// Add the rectangle to path
		//ERR(drawbotSuites->path_suiteP->AddRect(btn_path_ref, &btn));
		//
		//btn_X += btn_W + 4.0f;
		//btn.left += btn_W + 4.0f;
		//// Add the rectangle to path
		//ERR(drawbotSuites->path_suiteP->AddRect(btn_path_ref, &btn));
		//// Fill the path with the brush created
		//ERR(drawbotSuites->surface_suiteP->FillPath(surface_ref, normal_brush, btn_path_ref, kDRAWBOT_FillType_Default));
		//
		//
		//DRAWBOT_UTF16Char btn_string[16]; copyConvertStringLiteralIntoUTF16(L"+", btn_string);
		//DRAWBOT_PointF32 text_origin{
		//	static_cast<float>(event_extra->effect_win.current_frame.left + btn_W / 2.0f),
		//	static_cast<float>(event_extra->effect_win.current_frame.top + btn_H - 2.0f)
		//};
		//ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
		//	white_brush,
		//	font_ref,
		//	&btn_string[0],
		//	&text_origin,
		//	kDRAWBOT_TextAlignment_Center,
		//	kDRAWBOT_TextTruncation_None,
		//	0.0f));

		//copyConvertStringLiteralIntoUTF16(L"-", btn_string);
		//text_origin.x += btn_W + 4.0f;
		//ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
		//	white_brush,
		//	font_ref,
		//	&btn_string[0],
		//	&text_origin,
		//	kDRAWBOT_TextAlignment_Center,
		//	kDRAWBOT_TextTruncation_None,
		//	0.0f));


		AEGP_SuiteHandler	suites(in_data->pica_basicP);
		GradientInfo grad = *reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(params[COLORAMEN_GRADIENT]->u.arb_d.value));

		auto* p = suites.HandleSuite1()->host_lock_handle(out_data->global_data);

		DRAWBOT_ImageRef img = NULL;
		drawbotSuites->supplier_suiteP->NewImageFromBuffer(
			supplier_ref,
			COLORAMEN_GRADIENT_UI_WIDTH,
			1,
			COLORAMEN_GRADIENT_UI_WIDTH * 4,
			kDRAWBOT_PixelLayout_24BGR,
			p,
			&img
		);

		DRAWBOT_PointF32 img_origin {
			(float)event_extra->effect_win.current_frame.left,
			(float)event_extra->effect_win.current_frame.top
		};
		
		while (img_origin.y < (float)event_extra->effect_win.current_frame.top + 5.0f) {
			drawbotSuites->surface_suiteP->DrawImage(surface_ref, img, &img_origin, 1.0f);
			img_origin.y += 1.0f;
		}
		// release img
		if (img) {
			ERR2(drawbotSuites->supplier_suiteP->ReleaseObject(reinterpret_cast<DRAWBOT_ObjectRef>(img)));
		}
		
		suites.HandleSuite1()->host_unlock_handle(out_data->global_data);
	}



	if (!err) {
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;
	}

	return err;
}

PF_Err
HandleEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* extra)
{
	PF_Err		err = PF_Err_NONE;

	switch (extra->e_type) {
	case PF_Event_DO_CLICK:
		break;
	case PF_Event_DRAG:
		break;
	case PF_Event_DRAW:
		err = DrawEvent(in_data,
			out_data,
			params,
			output,
			extra);
		break;
	case PF_Event_ADJUST_CURSOR:
		break;
	default:
		break;
	}
	return err;
}