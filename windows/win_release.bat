::%QT_DIR% path of Qt installation
set PATH=%PATH%;%QT_DIR%\..\Tools\mingw730_32\bin
set windeployqt=%QT_DIR%\mingw73_32\bin\windeployqt.exe
set  exe_dir=%HOMEPATH%\Documents\QtChecksum\build-QtChecksum-Desktop_Qt_5_14_2_MinGW_32_bit-Release\release
set  to_dir=win_qtchecksum
set  to_exe=%to_dir%\QtChecksum.exe

cd %~dp0
rmdir /s /q %to_dir%
mkdir %to_dir%
copy %exe_dir%\*.exe %to_dir%
%windeployqt% --compiler-runtime --dir %to_dir%  %to_exe%
pause