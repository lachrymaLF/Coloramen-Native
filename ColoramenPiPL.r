#include "AEConfig.h"
#include "AE_EffectVers.h"

#ifndef AE_OS_WIN
	#include <AE_General.r>
#endif
	
resource 'PiPL' (16000) {
	{	/* array properties: 12 elements */
		/* [1] */
		Kind {
			AEEffect
		},
		/* [2] */
		Name {
			"Coloramen"
		},
		/* [3] */
		Category {
			"lachrymal.net"
		},
#ifdef AE_OS_WIN
	#ifdef AE_PROC_INTELx64
		CodeWin64X86 {"EffectMain"},
	#endif
#else
	#ifdef AE_OS_MAC
		CodeMacIntel64 {"EffectMain"},
		CodeMacARM64 {"EffectMain"},
	#endif
#endif
		/* [6] */
		AE_PiPL_Version {
			2,
			0
		},
		/* [7] */
		AE_Effect_Spec_Version {
			PF_PLUG_IN_VERSION,
			PF_PLUG_IN_SUBVERS
		},
		/* [8] */
		AE_Effect_Version {
			32769
			/* AE_Effect_Version =
			MAJOR_VERSION * 524288 + MINOR_VERSION * 32768 + BUG_VERSION * 2048 + STAGE_VERSION * 512 + BUILD_VERSION [enum value]
			*/
		},
		/* [9] */
		AE_Effect_Info_Flags {
			0
		},
		/* [10] */
		AE_Effect_Global_OutFlags {
		0x02008400 //50332160

		},
		AE_Effect_Global_OutFlags_2 {
		0x00000000
		},
		/* [11] */
		AE_Effect_Match_Name {
			"LKRM Coloramen"
		},
		/* [12] */
		AE_Reserved_Info {
			0
		}
	}
};

