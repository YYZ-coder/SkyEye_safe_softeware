
#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifdef __cplusplus
extern "C" 
{
#endif

BOOLEAN CompareUCharString(PUCHAR start,const PUCHAR string,ULONG length);
BOOLEAN CompareWCharString(PWCHAR start,PWCHAR string,ULONG length);
ULONG MyFileFullPathQuery(IN PFILE_OBJECT file,OUT PUNICODE_STRING path);
VOID  GetFileNameFromDirectory(PUNICODE_STRING directory,OUT PUNICODE_STRING filename);
BOOLEAN   Query_SymbolLinkInfo();

#ifdef __cplusplus
}
#endif