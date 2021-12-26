#pragma once

#ifndef COLORAMEN_H
#define COLORAMEN_H

typedef unsigned char		u_char;
typedef unsigned short		u_short;
typedef unsigned short		u_int16;
typedef unsigned long		u_long;
typedef short int			int16;
#define PF_TABLE_BITS	12
#define PF_TABLE_SZ_16	4096

#define PF_DEEP_COLOR_AWARE 1	// make sure we get 16bpc pixels; 
								// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
	typedef unsigned short PixelType;
	#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"

#include "Coloramen_Strings.h"

/* Versioning information */

#define	MAJOR_VERSION	1
#define	MINOR_VERSION	0
#define	BUG_VERSION		0
#define	STAGE_VERSION	PF_Stage_DEVELOP
#define	BUILD_VERSION	1


/* Parameter defaults */

#define	COLORAMEN_OPACITY_MIN		0
#define	COLORAMEN_OPACITY_MAX		100
#define	COLORAMEN_OPACITY_DFLT		100

enum {
	COLORAMEN_INPUT = 0,
	COLORAMEN_OPACITY,
	COLORAMEN_GRADIENT,
	COLORAMEN_NUM_PARAMS
};

enum {
	OPACITY_DISK_ID = 1,
	GRADIENT_DISK_ID
};

typedef struct OpacityInfo{
	PF_FpLong	opacityF;
} OpacityInfo, *OpacityInfoP, **OpacityInfoH;

#define COLORAMEN_MAX_TABS 16

#define COLORAMEN_GRAD_REFCON (void*)0xDEADBEEFDEADBEEF

typedef struct GradientInfo {
	u_char num_tabs;
	struct {
		PF_FpShort pos;
		PF_PixelFloat color;
	} tabs[COLORAMEN_MAX_TABS];
} GradientInfo;

#define COLORAMEN_GRADIENT_UI_WIDTH		100
#define COLORAMEN_GRADIENT_UI_HEIGHT	150

typedef struct Aggregate {
	OpacityInfo			opacity;
	GradientInfo		grad;
} Aggregate;

extern "C" {

	DllExport
	PF_Err
	EffectMain(
		PF_Cmd			cmd,
		PF_InData		*in_data,
		PF_OutData		*out_data,
		PF_ParamDef		*params[],
		PF_LayerDef		*output,
		void			*extra);

}

PF_Err
CreateDefaultGrad(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ArbitraryH* dephault);

PF_PixelFloat GetColorARGB(const PF_FpShort& pos, const GradientInfo& grad);

PF_Err
Grad_Copy(
	PF_InData* in_data,
	PF_OutData* out_data,
	const PF_ArbitraryH* srcP,
	PF_ArbitraryH* dstP);

PF_Err
HandleEvent(
	PF_InData* in_data,
	PF_OutData* out_data,
	PF_ParamDef* params[],
	PF_LayerDef* output,
	PF_EventExtra* extra);

#endif // COLORAMEN_H