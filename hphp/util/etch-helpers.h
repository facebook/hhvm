#ifndef incl_ETCH_HELPERS_H
#define incl_ETCH_HELPERS_H

#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
#define CFI(x)          .cfi_##x
#define CFI2(x, y)      .cfi_##x y
#define CFI3C(x, y, z)  .cfi_##x y##, z
#define ETCH_ALIGN16    .align 16
#define ETCH_ALIGN8     .align 8
#define ETCH_ALIGN4     .align 4
#define ETCH_SECTION(x) .section .text.x
#define ETCH_SIZE(x)    /* Not used with PE/COFF on windows */
#define ETCH_NAME(x)    x
#define ETCH_LABEL(x)   .L##x
#define ETCH_TYPE(x, y) /* Not used on Windows */
#define ETCH_NAME_REL(x) $ x
#elif defined(__APPLE__)
#define CFI(x)          .cfi_##x
#define CFI2(x, y)      .cfi_##x y
#define CFI3C(x, y, z)  .cfi_##x y##, z
#define ETCH_ALIGN16    .align 4 // on OSX this is 2^value
#define ETCH_ALIGN8     .align 3
#define ETCH_ALIGN4     .align 2
#define ETCH_SECTION(x) .text
#define ETCH_SIZE(x)    /* not used on OSX */
#define ETCH_NAME(x)    _##x
#define ETCH_LABEL(x)   .L##_##x
#define ETCH_TYPE(x, y) /* not used on OSX */
#define ETCH_NAME_REL(x) _##x@GOTPCREL(%rip)
#else
#define CFI(x)          .cfi_##x
#define CFI2(x, y)      .cfi_##x y
#define CFI3C(x, y, z)  .cfi_##x y##, z
#define ETCH_ALIGN16    .align 16
#define ETCH_ALIGN8     .align 8
#define ETCH_ALIGN4     .align 4
#define ETCH_SECTION(x) .section .text.x,"ax"
#define ETCH_SIZE(x)    .size x, .-x
#define ETCH_NAME(x)    x
#define ETCH_LABEL(x)   .L##x
#define ETCH_TYPE(x, y) .type x, y
#define ETCH_NAME_REL(x) $ x
#endif

#endif // incl_ETCH_HELPERS_H
