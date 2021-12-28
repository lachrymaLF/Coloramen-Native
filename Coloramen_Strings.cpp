#include "Coloramen.h"

typedef struct {
	A_u_long	index;
	A_char		str[256];
} TableString;



TableString		g_strs[StrID_NUMTYPES] = {
	StrID_NONE,						"",
	StrID_Name,						"Coloramen",
	StrID_Description,				"OLIVE GARDEN CONFIDENTIAL INTERNAL RELEASE ONLY\rlachrymal.net 2022",
	StrID_Opacity_Param_Name,		"Opacity",
	StrID_Gradient_Param_Name,		"Gradient",
};


char	*GetStringPtr(int strNum)
{
	return g_strs[strNum].str;
}
	