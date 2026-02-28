#pragma once

#ifdef __aarch64__
#define CFI(x)          .cfi_##x
#define CFI2(x, y)      .cfi_##x y
#define CFI3C(x, y, z)  .cfi_##x y##, z
#define ETCH_ALIGN16    .balign 16
#define ETCH_ALIGN8     .balign 8
#define ETCH_ALIGN4     .balign 4
#define ETCH_SECTION(x) .section .text.x,"ax"
#define ETCH_SIZE(x)    .size x, .-x
#define ETCH_NAME(x)    x
#define ETCH_LABEL(x)   .L##x
#define ETCH_TYPE(x, y) .type x, y
#define ETCH_NAME_REL(x) $ x
#define ETCH_ARG1       %x0
#define ETCH_ARG2       %x1
#define ETCH_ARG3       %x2
#define ETCH_ARG4       %x3
#define ETCH_ARG5       %x4
#define ETCH_ARG6       %x5

#else /* Other x86 (e.g. linux) */
#define CFI(x)            .cfi_##x
#define CFI2(x, y)        .cfi_##x y
#define CFI3C(x, y, z)    .cfi_##x y##, z
#define ETCH_ALIGN16      .align 16
#define ETCH_ALIGN8       .align 8
#define ETCH_ALIGN4       .align 4
#define ETCH_SECTION(x)   .section .text.x,"ax"
#define ETCH_SIZE(x)      .size x, .-x
#define ETCH_NAME(x)      x
#define ETCH_LABEL(x)     .L##x
#define ETCH_TYPE(x, y)   .type x, y
#define ETCH_NAME_REL(x)  $ x
#define ETCH_ARG1         %rdi
#define ETCH_ARG2         %rsi
#define ETCH_ARG3         %rdx
#define ETCH_ARG4         %rcx
#define ETCH_ARG5         %r8
#define ETCH_ARG6         %r9
#define ETCH_GET_ARG5     /* not used */
#define ETCH_GET_ARG6     /* not used */
#define ETCH_RET1         %rax
#endif
