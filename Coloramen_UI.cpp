#include "Coloramen.h"

#include "AEFX_SuiteHelper.h"
#include <adobesdk/DrawbotSuite.h>

#include <cwchar>

constexpr float btn_H = 24.0f;
constexpr float tab_width = 10.0f; // d
constexpr float tab_height = 25.0f; // d


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
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

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

	DRAWBOT_ColorRGBA normal_color = { 20.0f / PF_MAX_CHAN8, 20.0f / PF_MAX_CHAN8, 20.0f / PF_MAX_CHAN8, 1.0 };
	DRAWBOT_ColorRGBA highlight_color = { 200.0f / PF_MAX_CHAN8, 200.0f / PF_MAX_CHAN8, 200.0f / PF_MAX_CHAN8, 1.0 };
	DRAWBOT_ColorRGBA white = { 255.0f / PF_MAX_CHAN8, 255.0f / PF_MAX_CHAN8, 255.0f / PF_MAX_CHAN8, 1.0 };
	
	DRAWBOT_BrushP normal_brush(drawbotSuites->supplier_suiteP, supplier_ref, &normal_color);
	DRAWBOT_BrushP highlight_brush(drawbotSuites->supplier_suiteP, supplier_ref, &highlight_color);
	DRAWBOT_BrushP white_brush(drawbotSuites->supplier_suiteP, supplier_ref, &white);

	// Get the default font size.
	// Create default font with default size.  Note that you can provide a different font size.
	// float default_font_sizeF = 0.0;
	// ERR(drawbotSuites->supplier_suiteP->GetDefaultFontSize(supplier_ref, &default_font_sizeF));
	// DRAWBOT_FontP font_ref(drawbotSuites->supplier_suiteP, supplier_ref, default_font_sizeF);
	DRAWBOT_FontP font_ref(drawbotSuites->supplier_suiteP, supplier_ref, 40.0);
	DRAWBOT_FontP font_change_btn_ref(drawbotSuites->supplier_suiteP, supplier_ref, 15.0);

	if (PF_EA_CONTROL == event_extra->effect_win.area) {
		const float btn_X = 0.0f, btn_Y = 0.0f, btn_W = 24.0f, btn_change_W = 60.0f;
		{
			DRAWBOT_PathP btn_path_ref(drawbotSuites->supplier_suiteP, supplier_ref);
			DRAWBOT_RectF32	btn{
				static_cast<float>(event_extra->effect_win.current_frame.left + 0.5 + btn_X),	// Center of the pixel in new drawing model is (0.5, 0.5)
				static_cast<float>(event_extra->effect_win.current_frame.top + 0.5 + btn_Y),
				btn_W,
				btn_H
			};
			// Add the rectangle to path
			ERR(drawbotSuites->path_suiteP->AddRect(btn_path_ref, &btn));

			btn.left += btn_W + 4.0f;
			// Add the rectangle to path
			ERR(drawbotSuites->path_suiteP->AddRect(btn_path_ref, &btn));

			btn.left += btn_W + 4.0f;
			btn.width = btn_change_W;
			// Add the rectangle to path
			ERR(drawbotSuites->path_suiteP->AddRect(btn_path_ref, &btn));

			// Fill the path with the brush created
			ERR(drawbotSuites->surface_suiteP->FillPath(surface_ref, normal_brush, btn_path_ref, kDRAWBOT_FillType_Default));



			DRAWBOT_UTF16Char btn_string[64]; copyConvertStringLiteralIntoUTF16(L"+", btn_string);
			DRAWBOT_PointF32 text_origin{
				static_cast<float>(event_extra->effect_win.current_frame.left + btn_W / 2.0f - 2.0f),
				static_cast<float>(event_extra->effect_win.current_frame.top + btn_H - 2.0f)
			};
			ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
				white_brush,
				font_ref,
				&btn_string[0],
				&text_origin,
				kDRAWBOT_TextAlignment_Center,
				kDRAWBOT_TextTruncation_None,
				0.0f));

			copyConvertStringLiteralIntoUTF16(L"-", btn_string);
			text_origin.x += btn_W + 4.0f;
			ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
				white_brush,
				font_ref,
				&btn_string[0],
				&text_origin,
				kDRAWBOT_TextAlignment_Center,
				kDRAWBOT_TextTruncation_None,
				0.0f));

			copyConvertStringLiteralIntoUTF16(L"Change", btn_string);
			text_origin.x += btn_W + 25.0f;
			text_origin.y -= 5.0f;
			ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
				white_brush,
				font_change_btn_ref,
				&btn_string[0],
				&text_origin,
				kDRAWBOT_TextAlignment_Center,
				kDRAWBOT_TextTruncation_None,
				0.0f));

			copyConvertStringLiteralIntoUTF16(L"OLIVE GARDEN CONFIDENTIAL", btn_string);
			text_origin.x = (float)event_extra->effect_win.current_frame.left;
			text_origin.y += 70.0f;
			ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
				white_brush,
				font_change_btn_ref,
				&btn_string[0],
				&text_origin,
				kDRAWBOT_TextAlignment_Left,
				kDRAWBOT_TextTruncation_None,
				0.0f));
			copyConvertStringLiteralIntoUTF16(L"INTERNAL USE ONLY", btn_string);
			text_origin.y += 20.0f;
			ERR(drawbotSuites->surface_suiteP->DrawString(surface_ref,
				white_brush,
				font_change_btn_ref,
				&btn_string[0],
				&text_origin,
				kDRAWBOT_TextAlignment_Left,
				kDRAWBOT_TextTruncation_None,
				0.0f));
		}

		GradientInfo grad = *reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(params[COLORAMEN_GRADIENT]->u.arb_d.value));

		GlobalData* global_p = reinterpret_cast<GlobalData*>(suites.HandleSuite1()->host_lock_handle(out_data->global_data));
		unsigned char* p = &(global_p->img[0][0]);

		for (int i = 0; i < 4 * COLORAMEN_GRADIENT_BUFFER_WIDTH; i += 4) {
			auto c = GetColorARGB(float(i) / float(4 * COLORAMEN_GRADIENT_BUFFER_WIDTH), grad);
			*(p + i) = static_cast<unsigned char>(c.blue * PF_MAX_CHAN8);
			*(p + i + 1) = static_cast<unsigned char>(c.green * PF_MAX_CHAN8);
			*(p + i + 2) = static_cast<unsigned char>(c.red * PF_MAX_CHAN8);
			*(p + i + 3) = 255;
		}

		DRAWBOT_ImageRef img = NULL;
		ERR(drawbotSuites->supplier_suiteP->NewImageFromBuffer(
			supplier_ref,
			COLORAMEN_GRADIENT_BUFFER_WIDTH,
			1,
			COLORAMEN_GRADIENT_BUFFER_WIDTH * 4,
			kDRAWBOT_PixelLayout_32BGRA_Straight,
			p,
			&img
		));

		DRAWBOT_PointF32 img_origin {
			(float)event_extra->effect_win.current_frame.left + tab_width,
			(float)event_extra->effect_win.current_frame.top + btn_H + 3.0f
		};

		auto temp = img_origin.y;
		while (img_origin.y < temp + COLORAMEN_GRADIENT_BUFFER_HEIGHT) {
			ERR(drawbotSuites->surface_suiteP->DrawImage(surface_ref, img, &img_origin, 1.0f));
			img_origin.y += 1.0f;
		}


		DRAWBOT_ColorRGBA stroke_col{ 0.5, 0.5, 0.5, 1.0 };
		DRAWBOT_ColorRGBA stroke_outer_col{ 0, 0, 0, 1.0 };
		DRAWBOT_PenP pen_outer_ref(drawbotSuites->supplier_suiteP, supplier_ref, &stroke_outer_col, 2.0f);
		DRAWBOT_PenP pen_ref(drawbotSuites->supplier_suiteP, supplier_ref, &stroke_col, 1.0f);
		DRAWBOT_PenP pen_tab_white_ref(drawbotSuites->supplier_suiteP, supplier_ref, &white, 1.0f);
		for (int i = 0; i < grad.num_tabs; i++) {
			DRAWBOT_PathP tab_path(drawbotSuites->supplier_suiteP, supplier_ref);
			DRAWBOT_ColorRGBA tab_col{
				grad.tabs[i].color.red,
				grad.tabs[i].color.green,
				grad.tabs[i].color.blue,
				grad.tabs[i].color.alpha
			};
			DRAWBOT_BrushP tab_brush(drawbotSuites->supplier_suiteP, supplier_ref, &tab_col);

			DRAWBOT_RectF32	tab{
				static_cast<float>(event_extra->effect_win.current_frame.left + grad.tabs[i].pos * COLORAMEN_GRADIENT_BUFFER_WIDTH + tab_width / 2.0f),
				static_cast<float>(event_extra->effect_win.current_frame.top + btn_H + 3.0f + COLORAMEN_GRADIENT_BUFFER_HEIGHT * .9f),
				tab_width,
				tab_height
			};

			ERR(drawbotSuites->path_suiteP->AddRect(tab_path, &tab));
			ERR(drawbotSuites->surface_suiteP->FillPath(surface_ref, tab_brush, tab_path, kDRAWBOT_FillType_Default));
			ERR(drawbotSuites->surface_suiteP->StrokePath(surface_ref, pen_outer_ref, tab_path));
			ERR(drawbotSuites->surface_suiteP->StrokePath(surface_ref, pen_ref, tab_path));

			DRAWBOT_PathP tri_path(drawbotSuites->supplier_suiteP, supplier_ref);
			ERR(drawbotSuites->path_suiteP->MoveTo(tri_path, tab.left, tab.top));
			ERR(drawbotSuites->path_suiteP->LineTo(tri_path, tab.left + tab_width, tab.top));
			ERR(drawbotSuites->path_suiteP->LineTo(tri_path, tab.left + tab_width / 2.0f, tab.top - 8.0f));
			ERR(drawbotSuites->path_suiteP->Close(tri_path));
			ERR(drawbotSuites->surface_suiteP->FillPath(surface_ref, 
														i == grad.selected ? highlight_brush : normal_brush,
														tri_path, kDRAWBOT_FillType_Default));
			ERR(drawbotSuites->surface_suiteP->StrokePath(surface_ref, pen_outer_ref, tri_path));
			ERR(drawbotSuites->surface_suiteP->StrokePath(surface_ref, pen_tab_white_ref, tri_path));
		}


		// release img
		if (img) {
			ERR2(drawbotSuites->supplier_suiteP->ReleaseObject(reinterpret_cast<DRAWBOT_ObjectRef>(img)));
		}

		// unlock image buffer handle
		suites.HandleSuite1()->host_unlock_handle(out_data->global_data);
	}


	if (!err) {
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;
	}

	return err;
}

static PF_Err
DoClick(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	PF_Err	err = PF_Err_NONE;

	AEGP_SuiteHandler		suites(in_data->pica_basicP);

	auto gradH = params[COLORAMEN_GRADIENT]->u.arb_d.value;
	GradientInfo* grad = reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(gradH));
	PF_Point m_pos = {
		event_extra->u.do_click.screen_point.h - event_extra->effect_win.current_frame.left,
		event_extra->u.do_click.screen_point.v - event_extra->effect_win.current_frame.top 
	};

	PF_Point add_btn_pos{};
	PF_Point del_btn_pos{ 4 + 24, 0 };
	PF_Point change_btn_pos{ (4 + 24) * 2, 0};


	PF_Point tab_pos{};
	tab_pos.v = static_cast<A_long>(btn_H + 3.0f + COLORAMEN_GRADIENT_BUFFER_HEIGHT * .9f);

	if (m_pos.v > add_btn_pos.v && m_pos.v - add_btn_pos.v < btn_H) { // condition same for all btns currently
		if (m_pos.h > add_btn_pos.h && m_pos.h - add_btn_pos.h < 24) {
			AddTab(grad);
			params[COLORAMEN_GRADIENT]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
		}
		else if (m_pos.h > del_btn_pos.h && m_pos.h - del_btn_pos.h < 24) {
			DelSelectedTab(grad);
			params[COLORAMEN_GRADIENT]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
		}
		else if (m_pos.h > change_btn_pos.h && m_pos.h - change_btn_pos.h < 60) {
			PF_PixelFloat col{};
			ERR(suites.AppSuite6()->PF_AppColorPickerDialog(
				"Coloramen",
				&(grad->tabs[grad->selected].color),
				FALSE,
				&col));
			if (!err) {
				grad->tabs[grad->selected].color = col;
				params[COLORAMEN_GRADIENT]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
			}
		}

	}
	else if (m_pos.v > tab_pos.v && m_pos.v - tab_pos.v < tab_height)
		for (int i = 0; i < grad->num_tabs; i++) {
			tab_pos.h = static_cast<A_long>(grad->tabs[i].pos * COLORAMEN_GRADIENT_BUFFER_WIDTH + tab_width / 2.0f);
			if (m_pos.h > tab_pos.h && m_pos.h - tab_pos.h < tab_width) {
				grad->selected = i;
				params[COLORAMEN_GRADIENT]->uu.change_flags |= PF_ChangeFlag_CHANGED_VALUE;
				event_extra->u.do_click.send_drag = TRUE;
				break;
			}
		}

	suites.HandleSuite1()->host_unlock_handle(gradH);

	if (!err) {
		event_extra->evt_out_flags = PF_EO_HANDLED_EVENT;
	}
	return err;
}

// Only sent when we click on a tab
static PF_Err
DoDrag(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* event_extra)
{
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	PF_Err 			err = PF_Err_NONE;
	PF_ContextH		contextH = event_extra->contextH;
	PF_Point		mouse_down{};


	if (PF_Window_EFFECT == (*contextH)->w_type) {
		if (PF_EA_CONTROL == event_extra->effect_win.area) {
			if (!event_extra->u.do_click.last_time) {
				mouse_down = event_extra->u.do_click.screen_point;
				GradientInfo* grad = reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(params[COLORAMEN_GRADIENT]->u.arb_d.value));
				auto tmp = (mouse_down.h - event_extra->effect_win.current_frame.left - tab_width) / COLORAMEN_GRADIENT_BUFFER_WIDTH;
				grad->tabs[grad->selected].pos = static_cast<PF_FpShort>(MIN(MAX(tmp, 0.0), 1.0));
				
				if (grad->selected > 0 && grad->tabs[grad->selected].pos < grad->tabs[grad->selected - 1].pos) {
					SwapTabs(grad, grad->selected, grad->selected - 1);
					grad->selected--;
				}
				if (grad->selected < grad->num_tabs - 1 && grad->tabs[grad->selected].pos > grad->tabs[grad->selected + 1].pos) {
					SwapTabs(grad, grad->selected, grad->selected + 1);
					grad->selected++;
				}
				suites.HandleSuite1()->host_unlock_handle(params[COLORAMEN_GRADIENT]->u.arb_d.value);
				params[COLORAMEN_GRADIENT]->uu.change_flags = PF_ChangeFlag_CHANGED_VALUE;
			}
		}
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
		err = DoClick(in_data,
			out_data,
			params,
			output,
			extra);
		break;
	case PF_Event_DRAG:
		err = DoDrag(in_data,
			out_data,
			params,
			output,
			extra);
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
