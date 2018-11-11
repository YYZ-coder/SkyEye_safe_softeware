
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef __cplusplus
extern "C" 
{
#endif
VOID     AllocatePool(OUT POperationInformation &operationinformation,OUT POperationResult &result);
NTSTATUS VerifyControl(POperationInformation operationinformation,POperationResult result);

#ifdef __cplusplus
}
#endif