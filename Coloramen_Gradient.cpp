#include "Coloramen.h"

PF_Err
CreateDefaultGrad(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ArbitraryH* dephault)
{
	PF_Err			err = PF_Err_NONE;
	PF_Handle		grad_Handle = NULL;

	AEGP_SuiteHandler	suites(in_data->pica_basicP);

	grad_Handle = suites.HandleSuite1()->host_new_handle(sizeof(GradientInfo));

	if (grad_Handle) {
		GradientInfo* grad_Ptr = reinterpret_cast<GradientInfo*>(PF_LOCK_HANDLE(grad_Handle));
		if (!grad_Ptr) {
			err = PF_Err_OUT_OF_MEMORY;
		}
		else {
			AEFX_CLR_STRUCT(*grad_Ptr);

			grad_Ptr->num_tabs = 2;
			grad_Ptr->selected = 0;
			grad_Ptr->tabs[0].pos = 0.0;
			grad_Ptr->tabs[0].color = { 1.0, 0.0, 0.0, 0.0 };
			grad_Ptr->tabs[1].pos = 1.0;
			grad_Ptr->tabs[1].color = { 1.0, 1.0, 1.0, 1.0 };

			*dephault = grad_Handle;
		}
		suites.HandleSuite1()->host_unlock_handle(grad_Handle);
	}
	return err;

}

PF_Err
Grad_Copy(
	PF_InData* in_data,
	PF_OutData* out_data,
	const PF_ArbitraryH* srcP,
	PF_ArbitraryH* dstP)
{
	PF_Err err = PF_Err_NONE;

	PF_Handle	sourceH = *srcP;

	AEGP_SuiteHandler suites(in_data->pica_basicP);

	if (sourceH) {
		GradientInfo* src_arbP = reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(sourceH));
		if (!src_arbP) {
			err = PF_Err_OUT_OF_MEMORY;
		}
		else {
			PF_Handle	destH = *dstP;
			if (destH) {
				GradientInfo* dst_arbP = reinterpret_cast<GradientInfo*>(suites.HandleSuite1()->host_lock_handle(destH));
				if (!dst_arbP) {
					err = PF_Err_OUT_OF_MEMORY;
				}
				else {
					memcpy(dst_arbP, src_arbP, sizeof(GradientInfo));
					suites.HandleSuite1()->host_unlock_handle(destH);
				}
			}
			suites.HandleSuite1()->host_unlock_handle(sourceH);
		}
	}
	return err;
}

PF_Err
Grad_Compare(
	PF_InData* in_data,
	PF_OutData* out_data,
	const PF_ArbitraryH* a_arbP,
	const PF_ArbitraryH* b_arbP,
	PF_ArbCompareResult* resultP)
{
	PF_Err err = PF_Err_NONE;

	*resultP = PF_ArbCompare_EQUAL;

	if (*a_arbP && *b_arbP) {
		GradientInfo* a = reinterpret_cast<GradientInfo*>(PF_LOCK_HANDLE(*a_arbP));
		GradientInfo* b = reinterpret_cast<GradientInfo*>(PF_LOCK_HANDLE(*b_arbP));

		if (!a || !b) {
			err = PF_Err_UNRECOGNIZED_PARAM_TYPE;
		}
		else {
			if (a->num_tabs == b->num_tabs)
				for (int i = 0; i < a->num_tabs; i++)
				{
					if (a->tabs[i].pos == b->tabs[i].pos &&
						a->tabs[i].color.alpha == b->tabs[i].color.alpha &&
						a->tabs[i].color.red == b->tabs[i].color.red &&
						a->tabs[i].color.green == b->tabs[i].color.green &&
						a->tabs[i].color.blue == b->tabs[i].color.blue)
						continue;
					else
					{
						*resultP = PF_ArbCompare_NOT_EQUAL;
						break;
					}

				}

			PF_UNLOCK_HANDLE(*a_arbP);
			PF_UNLOCK_HANDLE(*b_arbP);
		}
	}
	return err;
}

PF_Err
Grad_Print(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ArbPrintFlags	print_flags,
	PF_ArbitraryH		arbH,
	A_u_long			print_sizeLu,
	A_char* print_bufferPC) 
{
	PF_Err		err = PF_Err_NONE;
	if (arbH) {
		GradientInfo* a_arbP;

		a_arbP = (GradientInfo*)PF_LOCK_HANDLE(arbH);

		if (!a_arbP) {
			err = PF_Err_UNRECOGNIZED_PARAM_TYPE;
		}
		else {
			if (!print_flags && print_sizeLu >= COLORAMEN_GRAD_MAX_PRINT_SIZE) {
				A_u_long		indexLu = 0;
				A_char			bufAC[64];

				PF_SPRINTF(bufAC, "Coloramen not yet implemented :mmtroll: lachrymal.net");

				ERR(AEFX_AppendText(bufAC, (const A_u_long)print_sizeLu, print_bufferPC, &indexLu));
			}
			else if ((print_sizeLu) && print_bufferPC) {
				print_bufferPC[0] = 0x00;
			}
			PF_UNLOCK_HANDLE(arbH);
		}
	}
	if (err == AEFX_ParseError_APPEND_ERROR) {
		err = PF_Err_OUT_OF_MEMORY;
	}
	return err;
}

PF_Err
Grad_Scan(
	PF_InData* in_data,
	PF_OutData* out_data,
	void* refconPV,
	const char* bufPC,
	unsigned long		bytes_to_scanLu,
	PF_ArbitraryH* arbPH)
{
	PF_Err err = PF_Err_NONE;

	return err;
}

template <typename T>
T lerp(const T& a, const T& b, const T& f)
{
	return a + f * (b - a);
}

PF_PixelFloat GetColorARGB(const PF_FpShort& pos, const GradientInfo& grad)
{
	PF_PixelFloat res{};
	if (pos < grad.tabs[0].pos)
		return grad.tabs[0].color;
	if (pos > grad.tabs[grad.num_tabs - 1].pos)
		return grad.tabs[grad.num_tabs - 1].color;
	for (int i = 0; i < grad.num_tabs; i++) {
		if (pos >= grad.tabs[i].pos && pos <= grad.tabs[i + 1].pos) {
			PF_FpShort fac = (pos - grad.tabs[i].pos) / (grad.tabs[i + 1].pos - grad.tabs[i].pos);
			res = {
				lerp<PF_FpShort>(grad.tabs[i].color.alpha, grad.tabs[i + 1].color.alpha, fac),
				lerp<PF_FpShort>(grad.tabs[i].color.red, grad.tabs[i + 1].color.red, fac),
				lerp<PF_FpShort>(grad.tabs[i].color.green, grad.tabs[i + 1].color.green, fac),
				lerp<PF_FpShort>(grad.tabs[i].color.blue, grad.tabs[i + 1].color.blue, fac)
			};
			break;
		}
	}
	return res;
}

void SwapTabs(GradientInfo* s, u_char a, u_char b)
{
	PF_FpShort temp_pos = s->tabs[a].pos;
	PF_PixelFloat temp_col = s->tabs[a].color;
	s->tabs[a].pos = s->tabs[b].pos;
	s->tabs[a].color = s->tabs[b].color;
	s->tabs[b].pos = temp_pos;
	s->tabs[b].color = temp_col;
}

void AddTab(GradientInfo* grad)
{
	if (grad->num_tabs < COLORAMEN_MAX_TABS)
	{
		PF_FpShort pos = (grad->tabs[0].pos + grad->tabs[1].pos) / 2.0f;
		PF_PixelFloat color = GetColorARGB(pos, *grad);
		for (int i = grad->num_tabs - 1; i > 0; i--)
		{
			SwapTabs(grad, i, i + 1);
		}
		grad->tabs[1].pos = pos;
		grad->tabs[1].color = color;
		grad->num_tabs++;
		grad->selected = 1;
	}
	else
		MessageBox(NULL, ":mmsus:", "Max number of tabs reached", MB_OK | MB_ICONINFORMATION);
}

void DelSelectedTab(GradientInfo* grad)
{
	if (grad->num_tabs > 2)
	{
		grad->tabs[grad->selected].color = {};
		grad->tabs[grad->selected].pos = {};
		for (int i = grad->selected; i < grad->num_tabs; i++)
		{
			SwapTabs(grad, i, i + 1);
		}
		grad->num_tabs--;

		if (grad->num_tabs == grad->selected)
			grad->selected--;
	}
	else
		MessageBox(NULL, ":mmsus:", "Cannot delete this tab", MB_OK | MB_ICONINFORMATION);
}

PF_Err
AEFX_AppendText(
	A_char* srcAC,				/* >> */
	const A_u_long			dest_sizeLu,		/* >> */
	A_char* destAC,			/* <> */
	A_u_long* current_indexPLu)	/* <> */
{
	PF_Err			err = PF_Err_NONE;

	A_u_long		new_strlenLu = (A_u_long)strlen(srcAC) + *current_indexPLu;


	if (new_strlenLu <= dest_sizeLu) {
		destAC[*current_indexPLu] = 0x00;

#ifdef AE_OS_WIN
		strncat_s(destAC, dest_sizeLu, srcAC, strlen(srcAC));
#else
		strncat(destAC, srcAC, strlen(srcAC));
#endif
		* current_indexPLu = new_strlenLu;
	}
	else {
		err = AEFX_ParseError_APPEND_ERROR;
	}

	return err;
}
