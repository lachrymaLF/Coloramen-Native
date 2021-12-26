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

template <typename T>
static T lerp(const T& a, const T& b, const T& f)
{
	return a + f * (b - a);
}

PF_PixelFloat GetColorARGB(const PF_FpShort& pos, const GradientInfo& grad)
{
	PF_PixelFloat res;
	AEFX_CLR_STRUCT(res);
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

