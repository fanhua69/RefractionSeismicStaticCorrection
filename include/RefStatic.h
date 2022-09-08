
#ifndef REF_STATIC_BASE_TAG
#define REF_STATIC_BASE_TAG

	#ifndef _COMPILING_REF_STATICBASE
		#if defined(_DEBUG) 
		#pragma comment(lib,"RefStaticD.lib") 
		//#pragma message("Automatically linking with SeisBaseD.dll") 
		#else 
		#pragma comment(lib,"RefStatic.lib") 
		//#pragma message("Automatically linking with SeisBase.dll") 
		#endif

		#define _REFSTATIC_EXT_CLASS  _declspec(dllimport)
	#else
		#define _REFSTATIC_EXT_CLASS  _declspec(dllexport)

	#endif

	#include "..\\resource.h"


#endif