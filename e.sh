# Flashes Cmd When Ran, i dunno why :((

i686-w64-mingw32-gcc -O2 LB3Decryptor.c -o LB3Decryptor.exe \
    -ladvapi32 -lshell32 -lshlwapi -luser32 -lmpr -lntdll

i686-w64-mingw32-strip LB3Decryptor.exe
