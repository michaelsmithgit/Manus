// Manus.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "Manus.h"


// This is an example of an exported variable
MANUS_API int nManus=0;

// This is an example of an exported function.
MANUS_API int fnManus(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see Manus.h for the class definition
CManus::CManus()
{
	return;
}
