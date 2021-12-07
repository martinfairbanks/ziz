@echo off

if not exist build mkdir build
pushd build

:: redirect output and error to NUL, 2 = stderr
del *.pdb >NUL 2>NUL

set exeNameDebug=win64_platform_debug
set buildFlags=-DDEVELOPER=1 -DHOTLOAD
set buildSubsystem=console
set compilerFlags=-Od -Oi -WX -Wall -MTd -FC -Z7 -TC -nologo
set linkerFlags=-incremental:no -opt:ref -subsystem:%buildSubsystem%
set linkerFlagsDll=-PDB:ziz_%random%.pdb -EXPORT:updateStateAndRender -EXPORT:initialize -EXPORT:cleanUp -EXPORT:onLoad

:: Hot load - 64-bit debug build with statically linked CRT and 3 translation units, exe, dll and assembly code
echo "WAITING FOR PDB ..." > lock.tmp
cl %compilerFlags% %buildFlags% ..\code\sketch.c  -Fe"ziz.dll" -LD -link %linkerFlagsDll% 
cl %compilerFlags% -c %buildFlags% ..\code\win64_platform.c
ml64 -c -Zi -nologo ..\code\assembly.asm
link -debug -nologo -opt:ref -subsystem:%buildSubsystem% win64_platform.obj assembly.obj -out:%exeNameDebug%.exe

del *.obj
del *.lib
del *.exp
del *.ilk

if not exist data mkdir data
copy ..\data\icon.ico data\

popd

