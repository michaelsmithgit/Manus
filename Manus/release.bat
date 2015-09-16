@echo off

set VERSION=%2%
set RELDIR=Manus-SDK-%VERSION%

cd %1%

echo %CD%

mkdir %RELDIR%
mkdir %RELDIR%\x64

copy .\x64\Release\Manus.dll .\%RELDIR%\x64
copy .\x64\Release\Manus.lib .\%RELDIR%\x64

copy .\Release\Manus.dll .\%RELDIR%
copy .\Release\Manus.lib .\%RELDIR%
copy .\Release\ManusTest.exe .\%RELDIR%

copy .\Manus\Manus.h .\%RELDIR%
copy .\ManusUnity\Manus.cs .\%RELDIR%

powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::CreateFromDirectory('%RELDIR%', '%RELDIR%.zip'); }"
move %RELDIR%.zip %RELDIR%