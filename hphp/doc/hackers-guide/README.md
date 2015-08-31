# Hacker's Guide to HHVM

This directory contains documentation on the internal architecture of HHVM,
targeted at C++ developers looking to hack on HHVM itself.

* HHBC spec
* Frontend
  * Parser
  * Emitter
  * HHBBC
  * ...
* VM Runtime
  * Runtime data structures
    * Unit
    * Func
    * PreClass/Class
    * SrcKey
  * Execution Context
  * [Bytecode interpreter](bytecode-interpreter.md)
  * Unwinder
  * Treadmill
  * Debugger
  * ...
* JIT Compiler
  * [Core concepts](jit-core.md)
  * [Optimization passes](jit-optimizations.md)
  * ...
