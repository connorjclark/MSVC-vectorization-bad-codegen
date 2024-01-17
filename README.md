# MSVC bug report

Running this in Debug vs Release mode for:

> Microsoft Visual Studio Community 2022 (64-bit) - Current
> Version 17.8.4

... results in different results due to bad code gen during auto
vectorization.


The generated code looks a bit like this:

```
00007FF92EFBC2A8  andnps      xmm0,xmm1  
00007FF92EFBC2AB  movdqu      xmm0,xmmword ptr [r11+rax*8]  
00007FF92EFBC2B1  movdqu      xmmword ptr [r8+rdi*8],xmm1
```

... where `andnps` is writing to register `xmm0`, but then instantly overwrites that value with something else
in the next instruction.

Originally discovered in the asmjit library: https://github.com/asmjit/asmjit/issues/427
