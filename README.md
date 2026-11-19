## LockBit Black Decryptor
Reconstructed Code of LockBit Black Decryptor.

For Educational Purposes only!

Logging Format;
```
true|<original path>|<new path>      → successfully decrypted
false|<path>|file empty              → empty file, skipped
false|<path>|file not found          → path disappeared
false|<path>|file not encrypted or damaged
false|<path>|file rsa key not valid  → footer key does not match
```

# Termux/Linux Compile
```bash
bash e.sh
```

# Warning
This is missing lots of stuff, this is only for education NOT fully Ready-for-use.

# Note
The compiled binary doesnt Run for some reason that i dunno.

If 11vm-mingw-w64 is missing the 32-bit runtime, link the system-provided i386 builtins;
```bash
mkdir -p $PREFIX/opt/llvm-mingw-w64/lib/clang/21/lib/i686-w64-windows-gnu
ln -sf $PREFIX/lib/clang/21/lib/windows/libclang_rt.builtins-i386.a \
       $PREFIX/opt/llvm-mingw-w64/lib/clang/21/lib/i686-w64-windows-gnu/libclang_rt.builtins.a
```

Im lazy to explain allat just read the code !??!?!

# Contact
Session;
```
05360e21fa52901681b01304f7fa9890b6f53f23e2867800f8403f475a4209b92f
```
