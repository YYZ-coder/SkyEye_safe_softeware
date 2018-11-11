
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef __cplusplus
extern "C" 
{
#endif

#define ROLE_SYSTEM 0x01
#define ROLE_ADVANCED 0x2
#define ROLE_COMMON   0x3
#define ROLE_GUEST   0x4

NTSTATUS  DBAC_ControlLogic(PUNICODE_STRING filepath,ULONG irp_type);
NTSTATUS  DBAC_ControlLogicEx(PUNICODE_STRING &directory,ULONG irp_type,PFILE_OBJECT fileobject);


#ifdef __cplusplus
}
#endif