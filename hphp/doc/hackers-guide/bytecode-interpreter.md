# The HHVM Bytecode Interpreter

A typical interpreter is implemented using a dispatch loop that looks something
like this:

```cpp
while (true) {
  switch(*pc) {
    case ADD:
      /* add implementation */
      pc++;
      break;

    case SUB:
      /* sub implementation */
      pc++;
      break;

    /*
     * handlers for other bytecodes
     */
  }
}
```

That is, you have a loop which steps through the bytecode program, examining
each one. The `switch` statement executes the current bytecode instruction,
advances the program counter, then repeats, until the program terminates.

If you try to find a code structure like this in HHVM, you won’t find it.
Instead, the interpreter is broken up into a number of smaller pieces, some of
which are defined using a series of nested macros. This is to reduce duplication
and to keep it more manageable, although it can make the code seem intimidating
to newcomers.

HHVM's interpreter makes repeated use of the [X
Macro](https://en.wikipedia.org/wiki/X_Macro) pattern to generate code and data
for each bytecode instruction. The list macro is `OPCODES`, defined in
[runtime/vm/hhbc.h](https://github.com/facebook/hhvm/blob/f484e7c597763bff68ad9e0e355aff763b71ec1e/hphp/runtime/vm/hhbc.h#L383).
It contains the name, signature, and attributes for every bytecode instruction.
The "X" macro is `O()`, which you can see is repeatedly invoked by `OPCODES`.

## Bytecode implementations

HHVM's bytecode interpreter lives in
[runtime/vm/bytecode.cpp](../../runtime/vm/bytecode.cpp). For every bytecode
instruction `Foo`, there is a function `iopFoo()`. This function contains the
interpreter implementation of `Foo`, and takes arguments representing all of the
immediate arguments to the instruction. For example, since the `Add` instruction
takes no immediates, `void iopAdd()` takes no arguments. `Int`, on the other
hand, takes a 64-bit integer immediate, so its signature is `void iopInt(int64_t
imm)`.

These functions are not called directly by the dispatch loop. Instead, the
dispatch loop calls `iopWrapFoo()`, giving it a pointer to the appropriate
`iopFoo()` function. These wrapper functions are automatically generated from
each bytecode's signature in `hhbc.h`. Each one decodes the appropriate
immediates, then passes them to the corresponding `iopFoo()` function. You
should not have to modify the machinery that generates these functions unless
you add a new bytecode immediate type.

## InterpOne

In addition to the hand-written `iop*()` functions and the macro-generated
`iopWrap*()` functions, the `OPCODES` macro is used
[here](https://github.com/facebook/hhvm/blob/f484e7c597763bff68ad9e0e355aff763b71ec1e/hphp/runtime/vm/bytecode.cpp#L7475-L7515)
to create a set of functions named `interpOne*()`. These functions are used by
the JIT when a certain instruction would be too complicated to compile to native
machine code: it syncs all live state to memory, calls the appropriate
`interpOne*()` function, then resumes running the native code after the
problematic bytecode.

## Interpreter dispatch loop

The dispatch loop is in `dispatchImpl()`, defined in
[runtime/vm/bytecode.cpp](../../runtime/vm/bytecode.cpp). It’s declared as:

```cpp
  template <bool breakOnCtlFlow> TCA dispatchImpl()
```

Two different versions are instantiated, one for each possible value of
`breakOnCtlFlow`. When `breakOnCtlFlow` is `true`, the function will return to
the caller after a control flow (i.e. branch) instruction is encountered. If
`breakOnCtlFlow` is `false`, the interpreter will continue executing
instructions until the current VM entry finishes.

There are two versions of the interpreter loop. The Windows version (indicated
with `_MSC_VER`) implements an ordinary switch-based interpreter loop, while the
Linux version implements a threaded interpreter. In a threaded interpreter, the
handler for each bytecode jumps directly to the handler for the next bytecode
rather than going to a single central switch statement. This eliminates a jump
to a different cache line and improves branch prediction by allowing the
processor’s branch predictor to find associations between the bytecodes. These
different mechanisms are hidden by the `DISPATCH_ACTUAL` macro.

There are three separate parts to each bytecode handler. One part for dealing
with Hack debugging, one part for tracking code coverage, and a third part which
implements the actual handler. These are defined in the `OPCODE_DEBUG_BODY`,
`OPCODE_COVER_BODY`, and `OPCODE_MAIN_BODY` macros, respectively. In the Windows
version, [defined
here](https://github.com/facebook/hhvm/blob/f484e7c597763bff68ad9e0e355aff763b71ec1e/hphp/runtime/vm/bytecode.cpp#L7621-L7628),
the three macros are put in a `case` for each opcode, like the example at the
beginning of this document.

In the Linux version, [defined
here](https://github.com/facebook/hhvm/blob/f484e7c597763bff68ad9e0e355aff763b71ec1e/hphp/runtime/vm/bytecode.cpp#L7631-L7636),
the threaded interpreter uses a dispatch table with [computed
goto](https://gcc.gnu.org/onlinedocs/gcc/Labels-as-Values.html). Each
instruction gets three labels: one for each `OPCODE_*_BODY` macro. These labels
are collected into three different dispatch tables, [defined
here](https://github.com/facebook/hhvm/blob/f484e7c597763bff68ad9e0e355aff763b71ec1e/hphp/runtime/vm/bytecode.cpp#L7532-L7549).
`optabDirect` points to `OPCODE_MAIN_BODY`, `optabCover` points to
`OPCODE_COVER_BODY`, and `optabDbg` points to `OPTAB_DEBUG_BODY`. The correct
dispatch table is selected at runtime depending on the current code coverage
configuration, and whether or not a debugger is attached.

## Performance

In general, we strongly prefer simplicity over performance in the interpreter.
Any performance-sensitive uses of HHVM rely on the JIT compiler, so we've found
it beneficial to keep the interpreter as straightforward as possible, kind of
like a reference implementation to compare the JIT against. This is an
intentional tradeoff that has resulted in an interpreter that is fairly slow.

One aspect of interpreter performance that we have focused on is inlining
decisions. We've found that most compilers choose to not inline various parts of
the interpreter in ways that measurably hurt performance (even when the
functions are marked `inline`). To counter this, we use the `OPTBLD_INLINE`
macro, which forces the compiler to inline functions in optimized builds. It does
nothing in debug builds, so we can still debug the interpreter.
