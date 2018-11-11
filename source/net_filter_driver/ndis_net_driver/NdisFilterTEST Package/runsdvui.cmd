cd /d "F:\VS2013_Drivers_prac\NdisFilterTEST Package" &msbuild "NdisFilterTEST Package.vcxproj" /t:sdvViewer /p:configuration="Win7 Debug" /p:platform=Win32
exit %errorlevel% 