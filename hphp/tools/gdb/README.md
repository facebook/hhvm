GDB Utilities for HHVM
======================

This directory contains a collection of Python-GDB utilities for HHVM
debugging, catered in particular towards debugging with a core, when no live
inferior is present.

To make use of them, simply execute the following line when running GDB, or add
it to your ~/.gdbinit (substituting the path to your hphp directory as
appropriate):

    source <path-to-hphp>/tools/gdb/hhvm.py

A brief overview of the supported commands and utility functions appears below.
Usage for commands can be obtained by running

    (gdb) help <command>


## deref

The `$deref` convenience function can be used to fully dereference a value,
stripping away all layers of `*`, `&`, and all supported smart pointer
wrappers.  Qualifiers (const/volatile) are also stripped.

    (gdb) whatis function.m_data.pobj->m_cls
    type = HPHP::LowClassPtr

    (gdb) p/r $deref(function.m_data.pobj->m_cls)
    $1 = {
      <HPHP::AtomicCountable> = {
        m_count = {
          <std::__atomic_base<int>> = {
            _M_i = 1
          }, <No data fields>}
      },
      members of HPHP::Class:
    ---Type <return> to continue, or q <return> to quit---


## idx

The `idx` command, and corresponding `$idx` convenience function, can be used
to index into arbitrary container objects, such as `std::unordered_map`.  See
idx.py for a list of supported containers.

    (gdb) idx jit::mcg->m_fixupMap.m_fixups 0x1b0e0000
    $14 = (HPHP::jit::FixupMap::FixupEntry *) 0x7f4153408018
    {
      firstElem = 838,
      fixup = {
        pcOffset = 838,
        spOffset = 15
      },
      indirect = {
        magic = 838,
        returnIpDisp = 15
      }
    }


## nameof

The `nameof` command prints the string names of named VM metadata objects, like
Funcs or Classes.

    (gdb) p $1
    $2 = (const HPHP::Func *) 0x2b83e520
    (gdb) nameof $1
    "ServiceMemoizer::__call_async"


## unit

The `unit` command sets or prints the current context Unit.  This is primarily
used for printing out literal strings without having to supply an explicit Unit
argument.

    (gdb) p $1
    $2 = (const HPHP::Func *) 0x2b83e520
    (gdb) p $1->m_unit
    $3 = (HPHP::Unit *) 0x2b7ee680
    (gdb) unit $3
    (gdb) unit
    $4 = (const HPHP::Unit *) 0x2b7ee680


## lookup

The `lookup` supercommand allows for looking up various VM objects by ID.
Currently supported flavors include `lookup func` and `lookup litstr`, the
latter of which requires a second Unit argument, or an already-set current Unit
(via the `unit` command).

The `lookup func` command also comes as a convenience function called
`$lookup_func`.

    (gdb) lookup func 44766
    $1 = (const HPHP::Func *) 0x2b83e520
    (gdb) p $1->m_funcId
    $2 = 44766


## walkstk

The `walkstk` command prints out the native stack interleaved with the PHP
stack, incorporating code and stack pointers, function names, and filepaths.

    (gdb) walkstk
    #0  0x7fff52a59250 @ 0x7f41eece6499: __GI_raise() at ../nptl/sysdeps/unix/sysv/linux/raise.c:56
    #1  {inline frame} @ 0x7f41eecdf113: __GI_abort at abort.c:90
    #2  0x7fff52a592e0 @ 0x7f41eecdf113: __assert_fail_base() at assert.c:92
    #3  0x7fff52a59310 @ 0x7f41eecdf1c3: __GI___assert_fail() at assert.c:101
    #4  0x7fff52a59550 @ 0x3290f7d: offsetOf() at hphp/.../vm/unit-inl.h:167
    #5  {inline frame} @ 0x2e84b55: HPHP::createBacktrace at hphp/.../base/backtrace.cpp:110
    #6  0x7fff52a596b0 @ 0x2e84b55: HPHP::debug_string_backtrace() at hphp/.../std/ext_std_errorfunc.cpp:103
    #7  0x7fff52a59cf0 @ 0x3495199: HPHP::bt_handler() at hphp/.../base/crash-reporter.cpp:81
    #8  0x7fff52a5a3d0 @ 0x7f41eece6520: __restore_rt()
    #9  {inline frame} @ 0x7f41eecdf113: __GI_raise at ../nptl/sysdeps/unix/sysv/linux/raise.c:56
    #10 {inline frame} @ 0x7f41eecdf113: __GI_abort at abort.c:90
    #11 0x7fff52a5a460 @ 0x7f41eecdf113: __assert_fail_base() at assert.c:92
    #12 0x7fff52a5a490 @ 0x7f41eecdf1c3: __GI___assert_fail() at assert.c:101
    #13 0x7fff52a5a740 @ 0x38fe9cd: HPHP::jit::irgen::(anonymous namespace)::emitBaseLCR() at hphp/.../jit/irgen-minstr.cpp:625
    #14 0x7fff52a5adf0 @ 0x3916127: emitBaseOp() at hphp/.../jit/irgen-minstr.cpp:754
    #15 {inline frame} @ 0x3bbae8f: emitMPre at hphp/.../jit/irgen-minstr.cpp:1242
    #16 {inline frame} @ 0x3bbae8f: HPHP::jit::irgen::(anonymous namespace)::implMInstr at hphp/.../jit/irgen-minstr.cpp:2067
    #17 {inline frame} @ 0x3bbae8f: HPHP::jit::irgen::emitCGetM at hphp/.../jit/irgen-minstr.cpp:2077
    #18 0x7fff52a5af80 @ 0x3bbae8f: translateDispatch() at hphp/.../jit/translator.cpp:1886
    #19 {inline frame} @ 0x3bc11b5: HPHP::jit::translateInstr at hphp/.../jit/translator.cpp:1963
    #20 0x7fff52a5b6a0 @ 0x3bc11b5: HPHP::jit::(anonymous namespace)::translateRegionImpl() at hphp/.../jit/translator.cpp:2361
    #21 0x7fff52a5bdc0 @ 0x3bc0c60: HPHP::jit::(anonymous namespace)::translateRegionImpl() at hphp/.../jit/translator.cpp:2287
    #22 0x7fff52a5c190 @ 0x3bc31c7: HPHP::jit::translateRegion() at hphp/.../jit/translator.cpp:2450
    #23 0x7fff52a5c920 @ 0x3a0048a: HPHP::jit::MCGenerator::translateWork() at hphp/.../jit/mc-generator.cpp:1688
    #24 0x7fff52a5c9b0 @ 0x3a0336b: HPHP::jit::MCGenerator::translate() at hphp/.../jit/mc-generator.cpp:476
    #25 0x7fff52a5cac0 @ 0x3a0a92b: HPHP::jit::MCGenerator::retranslateOpt() at hphp/.../jit/mc-generator.cpp:249
    #26 0x7fff52a5cb80 @ 0x3a0c4ce: HPHP::jit::MCGenerator::handleServiceRequest() at hphp/.../jit/mc-generator.cpp:1234
    #27 0x7fff52a5cc60 @ 0x3a0d49e: HPHP::jit::MCGenerator::enterTC() at hphp/.../jit/mc-generator.cpp:1093
    #28 0x7fff52a5cca0 @ 0x3ebe7ac: enterTCAfterPrologue() at hphp/.../jit/mc-generator.h:257
    #29 {inline frame} @ 0x3ebebf6: HPHP::ExecutionContext::enterVMAtAsyncFunc at hphp/.../vm/bytecode.cpp:1855
    #30 0x7fff52a5cd00 @ 0x3ebebf6: HPHP::ExecutionContext::enterVM() at hphp/.../vm/bytecode.cpp:1951
    #31 0x7fff52a5cd50 @ 0x3ec3ea9: HPHP::ExecutionContext::resumeAsyncFunc() at hphp/.../vm/bytecode.cpp:2241
    #32 {inline frame} @ 0x2c01315: HPHP::c_AsyncFunctionWaitHandle::resume at hphp/.../asio/async_function_wait_handle.cpp:122
    #33 0x7fff52a5cdc0 @ 0x2c01315: resume() at hphp/.../asio/resumable_wait_handle-defs.h:30
    #34 {inline frame} @ 0x2c290eb: HPHP::AsioContext::runUntil at hphp/.../asio/asio_context.cpp:130
    #35 0x7fff52a5ce10 @ 0x2c290eb: HPHP::c_WaitableWaitHandle::join() at hphp/.../asio/waitable_wait_handle.cpp:94
    #36 0x7fff52a5ce50 @ 0x2c281cb: HPHP::c_WaitHandle::t_join() at hphp/.../asio/wait_handle.cpp:68
    #37 0x7fff52a5ce80 @ 0x2547d25: HPHP::tg_10WaitHandle_join() at hphp/.../idl/asio.ext_hhvm.cpp:287
    #38 0x7f40f7a7fc40 @ 0xc013ff2: [PHP] HH\WaitHandle::join()
    #38 0x7f40f7a7fcc0 @ 0xb04be8b: [PHP] prep()
    #38 0x7f40f7a7fda0 @ 0xc13c03e: [PHP] Logger::log()
    #38 0x7f40f7a7fde0 @ 0xc043b45: [PHP] LoggerConfig::log()
    #38 0x7f40f7a7fe50 @ 0x125971b9: [PHP] WebBaseController::__invoke()
    #38 0x7f40f458c530 @ 0xc1136b4: [PHP] PSP::__invoke()
    #38 0x7fff52a5d2a0 @ 0xb000010: <unknown>
    #39 {inline frame} @ 0x35a88f9: enterTCHelper at hphp/.../jit/translator-asm-helpers.S:66
    #40 0x7fff52a5d2e0 @ 0x35a88f9: HPHP::jit::x64::BackEnd::enterTCHelper() at hphp/.../jit/back-end-x64.cpp:118
    #41 0x7fff52a5d3c0 @ 0x3a0d0d4: HPHP::jit::MCGenerator::enterTC() at hphp/.../jit/mc-generator.cpp:1064
    #42 0x7fff52a5d400 @ 0x3ebe7ac: enterTCAfterPrologue() at hphp/.../jit/mc-generator.h:257
    #43 {inline frame} @ 0x3ebebf6: HPHP::ExecutionContext::enterVMAtAsyncFunc at hphp/.../vm/bytecode.cpp:1855
    #44 0x7fff52a5d460 @ 0x3ebebf6: HPHP::ExecutionContext::enterVM() at hphp/.../vm/bytecode.cpp:1951
    #45 0x7fff52a5d4b0 @ 0x3ec3ea9: HPHP::ExecutionContext::resumeAsyncFunc() at hphp/.../vm/bytecode.cpp:2241
    #46 {inline frame} @ 0x2c01315: HPHP::c_AsyncFunctionWaitHandle::resume at hphp/.../asio/async_function_wait_handle.cpp:122
    #47 0x7fff52a5d520 @ 0x2c01315: resume() at hphp/.../asio/resumable_wait_handle-defs.h:30
    #48 {inline frame} @ 0x2c290eb: HPHP::AsioContext::runUntil at hphp/.../asio/asio_context.cpp:130
    #49 0x7fff52a5d570 @ 0x2c290eb: HPHP::c_WaitableWaitHandle::join() at hphp/.../asio/waitable_wait_handle.cpp:94
    #50 0x7fff52a5d5b0 @ 0x2c281cb: HPHP::c_WaitHandle::t_join() at hphp/.../asio/wait_handle.cpp:68
    #51 0x7fff52a5d5e0 @ 0x2547d25: HPHP::tg_10WaitHandle_join() at hphp/.../idl/asio.ext_hhvm.cpp:287
    #52 0x7f40f7a7fe80 @ 0xc013ff2: [PHP] HH\WaitHandle::join()
    #52 0x7f40f7a7ff00 @ 0xb01fd6f: [PHP] prep()
    #52 0x7f40f7a7ff70 @ 0x12532489: [PHP] PSP::run()
    #52 0x7f40f7a7ffc0 @ 0x1252b3ac: [PHP] PSP::__invoke()
    #52 0x7fff52a5da00 @ 0xb000010: <unknown>
    #53 {inline frame} @ 0x35a88f9: enterTCHelper at hphp/.../jit/translator-asm-helpers.S:66
    #54 0x7fff52a5da40 @ 0x35a88f9: HPHP::jit::x64::BackEnd::enterTCHelper() at hphp/.../jit/back-end-x64.cpp:118
    #55 0x7fff52a5db20 @ 0x3a0d0d4: HPHP::jit::MCGenerator::enterTC() at hphp/.../jit/mc-generator.cpp:1064
    #56 {inline frame} @ 0x3ebec46: enterTCAfterPrologue at hphp/.../jit/mc-generator.h:257
    #57 {inline frame} @ 0x3ebec46: HPHP::ExecutionContext::enterVMAtFunc at hphp/.../vm/bytecode.cpp:1891
    #58 0x7fff52a5db80 @ 0x3ebec46: HPHP::ExecutionContext::enterVM() at hphp/.../vm/bytecode.cpp:1949
    #59 0x7fff52a5dcd0 @ 0x3ebf52b: HPHP::ExecutionContext::invokeFunc() at hphp/.../vm/bytecode.cpp:2116
    #60 0x7fff52a5dd60 @ 0x33fae19: HPHP::vm_call_user_func() at hphp/.../base/builtin-functions.cpp:356
    #61 0x7fff52a5de50 @ 0x33dab7b: HPHP::ExecutionContext::executeFunctions() at hphp/.../base/execution-context.cpp:574
    #62 0x7fff52a5deb0 @ 0x33e246c: HPHP::ExecutionContext::onShutdownPostSend() at hphp/.../base/execution-context.cpp:598
    #63 0x7fff52a5e0a0 @ 0x32bacaf: HPHP::HttpRequestHandler::executePHPRequest() at hphp/.../server/http-request-handler.cpp:483
    #64 0x7fff52a5e430 @ 0x32bc8f6: HPHP::HttpRequestHandler::handleRequest() at hphp/.../server/http-request-handler.cpp:329
    #65 0x7fff52a606f0 @ 0x346ae25: run() at hphp/.../server/server.h:112
    #66 {inline frame} @ 0x346ccaf: start_server at hphp/.../base/program-functions.cpp:885
    #67 {inline frame} @ 0x346ccaf: HPHP::execute_program_impl at hphp/.../base/program-functions.cpp:1569
    #68 0x7fff52a60750 @ 0x346ccaf: HPHP::execute_program() at hphp/.../base/program-functions.cpp:956
    #69 0x7fff52a60830 @ 0x1eefac0: main() at hphp/.../hhvm/main.cpp:611
