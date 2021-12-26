#include "Coloramen.h"

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	
	suites.ANSICallbacksSuite1()->sprintf(	out_data->return_msg,
											"%s v%d.%d\r%s",
											STR(StrID_Name), 
											MAJOR_VERSION, 
											MINOR_VERSION, 
											STR(StrID_Description));
	return PF_Err_NONE;
}

static PF_Err 
GlobalSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	out_data->my_version = PF_VERSION(	MAJOR_VERSION, 
										MINOR_VERSION,
										BUG_VERSION, 
										STAGE_VERSION, 
										BUILD_VERSION);

	out_data->out_flags =	PF_OutFlag_DEEP_COLOR_AWARE |
							PF_OutFlag_CUSTOM_UI |
							PF_OutFlag_PIX_INDEPENDENT;	// just 16bpc, not 32bpc
	

	// auto p = new unsigned char[10][4];
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	out_data->global_data = suites.HandleSuite1()->host_new_handle(sizeof(unsigned char) * COLORAMEN_GRADIENT_UI_WIDTH * 4);
	unsigned char* p = reinterpret_cast<unsigned char*>(suites.HandleSuite1()->host_lock_handle(out_data->global_data));

	for (int i = 0; i < 4 * COLORAMEN_GRADIENT_UI_WIDTH; i += 4) {
		*(p+i) = 255;
		*(p+i+1) = 255;
		*(p+i+2) = 255;
		*(p+i+3) = 255;
	}

	suites.HandleSuite1()->host_unlock_handle(out_data->global_data);

	return PF_Err_NONE;
}

static PF_Err
GlobalSetdown(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output)
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	suites.HandleSuite1()->host_dispose_handle(out_data->global_data);
	return PF_Err_NONE;
}

static PF_Err 
ParamsSetup (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err		err		= PF_Err_NONE;
	PF_ParamDef	def;	

	AEFX_CLR_STRUCT(def);

	PF_ADD_FLOAT_SLIDERX(	STR(StrID_Opacity_Param_Name), 
							COLORAMEN_OPACITY_MIN,
							COLORAMEN_OPACITY_MAX,
							COLORAMEN_OPACITY_MIN,
							COLORAMEN_OPACITY_MAX,
							COLORAMEN_OPACITY_DFLT,
							PF_Precision_HUNDREDTHS,
							0,
							0,
							OPACITY_DISK_ID);

	AEFX_CLR_STRUCT(def);

	ERR(CreateDefaultGrad(	in_data,
							out_data,
							&def.u.arb_d.dephault));

	PF_ADD_ARBITRARY2(	STR(StrID_Gradient_Param_Name),
						COLORAMEN_GRADIENT_UI_WIDTH,
						COLORAMEN_GRADIENT_UI_HEIGHT,
						0,
						PF_PUI_CONTROL | PF_PUI_DONT_ERASE_CONTROL,
						def.u.arb_d.dephault,
						GRADIENT_DISK_ID,
						COLORAMEN_GRAD_REFCON);

	if (!err) {
		PF_CustomUIInfo			ci;

		AEFX_CLR_STRUCT(ci);

		ci.events = PF_CustomEFlag_EFFECT;

		ci.comp_ui_width = 0;
		ci.comp_ui_height = 0;
		ci.comp_ui_alignment = PF_UIAlignment_NONE;

		ci.layer_ui_width = 0;
		ci.layer_ui_height = 0;
		ci.layer_ui_alignment = PF_UIAlignment_NONE;

		ci.preview_ui_width = 0;
		ci.preview_ui_height = 0;
		ci.layer_ui_alignment = PF_UIAlignment_NONE;

		err = (*(in_data->inter.register_ui))(in_data->effect_ref, &ci);
	}


	out_data->num_params = COLORAMEN_NUM_PARAMS;

	return err;
}

static PF_Err
MySimpleGainFunc16 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel16	*inP, 
	PF_Pixel16	*outP)
{
	PF_Err		err = PF_Err_NONE;

	OpacityInfo	*giP	= reinterpret_cast<OpacityInfo*>(refcon);
	PF_FpLong	tempF	= 0;
					
	if (giP){
		tempF = giP->opacityF * PF_MAX_CHAN16 / 100.0;
		if (tempF > PF_MAX_CHAN16){
			tempF = PF_MAX_CHAN16;
		};

		outP->alpha		=	inP->alpha;
		outP->red		=	MIN((inP->red	+ (A_u_char) tempF), PF_MAX_CHAN16);
		outP->green		=	MIN((inP->green	+ (A_u_char) tempF), PF_MAX_CHAN16);
		outP->blue		=	MIN((inP->blue	+ (A_u_char) tempF), PF_MAX_CHAN16);
	}

	return err;
}

static PF_Err
DoIt8 (
	void		*refcon, 
	A_long		xL, 
	A_long		yL, 
	PF_Pixel8	*inP, 
	PF_Pixel8	*outP)
{
	PF_Err		err = PF_Err_NONE;

	Aggregate* ag	= reinterpret_cast<Aggregate*>(refcon);
	PF_FpLong	tempF	= 0;
					
	if (ag) {
		tempF = MIN(ag->opacity.opacityF * PF_MAX_CHAN8 / 100.0, PF_MAX_CHAN8);

		PF_FpLong fac = MIN(1.0, (0.2162 * inP->red + 0.7152 * inP->green + 0.0722 * inP->blue) / PF_MAX_CHAN8);

		auto temp = GetColorARGB((PF_FpShort)fac, ag->grad);

		outP->alpha		=	inP->alpha == 0 ? 0 : (A_u_char) (temp.alpha * PF_MAX_CHAN8);
		outP->red		=	(A_u_char) (temp.red * PF_MAX_CHAN8);
		outP->green		=	(A_u_char) (temp.green * PF_MAX_CHAN8);
		outP->blue		=	(A_u_char) (temp.blue * PF_MAX_CHAN8);
	}

	return err;
}

static PF_Err
HandleArbitrary(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_ArbParamsExtra* extra)
{
	PF_Err 	err = PF_Err_NONE;
	void* srcP = NULL,
		* dstP = NULL;

	switch (extra->which_function) {
	case PF_Arbitrary_NEW_FUNC:
		if (extra->u.new_func_params.refconPV != COLORAMEN_GRAD_REFCON) {
			err = PF_Err_INTERNAL_STRUCT_DAMAGED;
		}
		else {
			err = CreateDefaultGrad(in_data,
				out_data,
				extra->u.new_func_params.arbPH);
		}
		break;
	case PF_Arbitrary_DISPOSE_FUNC:
		if (extra->u.dispose_func_params.refconPV != COLORAMEN_GRAD_REFCON) {
			err = PF_Err_INTERNAL_STRUCT_DAMAGED;
		}
		else {
			PF_DISPOSE_HANDLE(extra->u.dispose_func_params.arbH);
		}
		break;
	case PF_Arbitrary_COPY_FUNC:		
		if (extra->u.copy_func_params.refconPV == COLORAMEN_GRAD_REFCON) {
			ERR(CreateDefaultGrad(in_data,
				out_data,
				extra->u.copy_func_params.dst_arbPH));

			ERR(Grad_Copy(in_data,
				out_data,
				&extra->u.copy_func_params.src_arbH,
				extra->u.copy_func_params.dst_arbPH));
		}
		break;
	case PF_Arbitrary_FLAT_SIZE_FUNC:
		break;
	case PF_Arbitrary_FLATTEN_FUNC:
		break;
	case PF_Arbitrary_UNFLATTEN_FUNC:
		break;
	case PF_Arbitrary_INTERP_FUNC:
		break;
	case PF_Arbitrary_COMPARE_FUNC:
		break;
	case PF_Arbitrary_PRINT_SIZE_FUNC:
		break;
	case PF_Arbitrary_PRINT_FUNC:
		break;
	case PF_Arbitrary_SCAN_FUNC:
		break;
	}
	return err;
}

static PF_Err 
Render (
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	PF_Err				err		= PF_Err_NONE;
	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	/*	Put interesting code here. */
	Aggregate aggregate;

	AEFX_CLR_STRUCT(aggregate);
	A_long				linesL	= 0;

	linesL 		= output->extent_hint.bottom - output->extent_hint.top;
	aggregate.opacity.opacityF 	= params[COLORAMEN_OPACITY]->u.fs_d.value;
	auto gradH = params[COLORAMEN_GRADIENT]->u.arb_d.value;
	aggregate.grad = *reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(gradH));
	
	if (PF_WORLD_IS_DEEP(output)){
		ERR(suites.Iterate16Suite1()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												&params[COLORAMEN_INPUT]->u.ld,	// src 
												NULL,							// area - null for all pixels
												(void*)&aggregate,					// refcon - your custom data pointer
												MySimpleGainFunc16,				// pixel function pointer
												output));
	} else {
		ERR(suites.Iterate8Suite1()->iterate(	in_data,
												0,								// progress base
												linesL,							// progress final
												&params[COLORAMEN_INPUT]->u.ld,	// src 
												NULL,							// area - null for all pixels
												(void*)&aggregate,					// refcon - your custom data pointer
												DoIt8,				// pixel function pointer
												output));	
	}
	PF_UNLOCK_HANDLE(gradH);
	return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		"Coloramen", // Name
		"LKRM Coloramen", // Match Name
		"lachrymal.net", // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:
				err = About(in_data, out_data, params, output);
				break;
			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(in_data, out_data, params, output);
				break;
			case PF_Cmd_GLOBAL_SETDOWN:
				err = GlobalSetdown(in_data, out_data, params, output);
				break;
			case PF_Cmd_PARAMS_SETUP:
				err = ParamsSetup(in_data, out_data, params, output);
				break;
			case PF_Cmd_RENDER:
				err = Render(in_data, out_data, params, output);
				break;
			case PF_Cmd_EVENT:
				err = HandleEvent(in_data, out_data, params, output, reinterpret_cast<PF_EventExtra*>(extra));
				break;
			case PF_Cmd_ARBITRARY_CALLBACK:
				err = HandleArbitrary(in_data, out_data, params, output, reinterpret_cast<PF_ArbParamsExtra*>(extra));
				break;
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

