// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MANUS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MANUS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MANUS_EXPORTS
#define MANUS_API __declspec(dllexport)
#else
#define MANUS_API __declspec(dllimport)
#endif

// This class is exported from the Manus.dll
class MANUS_API CManus {
public:
	CManus(void);
	// TODO: add your methods here.
};

extern MANUS_API int nManus;

MANUS_API int fnManus(void);
