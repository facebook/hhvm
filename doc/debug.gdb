
<h2>Useful gdb Commands</h2>

  info threads
  thread apply [threadno] [all] args
  set follow-fork-mode parent
  set detach-on-fork on
  set print pretty
  handle SIGPIPE nostop noprint pass
  dump binary memory [filename] [start_addr] [end_addr]
  symbol-file [un-stripped binary OR symbol file from strip]

=  b main.no.cpp:55

If you get "Couldn't get registers" error when starting a program with gdb,
instead of attaching to a running process, this is because gdb has a bug,
not able to switch to the main thread/process when it forks:

  [Thread debugging using libthread_db enabled]
  [New Thread 46912496246512 (LWP 30324)]
  [New Thread 1084229952 (LWP 30327)]
  [Thread 1084229952 (LWP 30327) exited]
  Couldn't get registers: No such process.

Set a break point at line 55 of main.no.cpp, then "r", you will get this,

  [Thread debugging using libthread_db enabled]
  [New Thread 46912496246512 (LWP 30632)]
  [New Thread 1084229952 (LWP 30636)]
  [Thread 1084229952 (LWP 30636) exited]
  <b>[Switching to Thread 46912496246512 (LWP 30632)]</b>

  Breakpoint 1, main (argc=3, argv=0x7fff41b5b138) at sys/main.no.cpp:55
  55        return HPHP::execute_program(argc, argv);
  (gdb) c

Magically, gdb is able to switch to main thread, attaching to it, then it will
be able to debug it, even if it forks afterwards.

<h2> Getting PHP symbols in the JIT under gdb </h2>

The VM periodically emits DWARF files containing function address
information for the JIT code it generates. These DWARF files are synced with gdb
asynchronously (by default every ~128 tracelets). This means that the backtrace
you see under gdb  may contain some unresolved PHP symbols that show up as ??s,
for symbols that have not been synced.

There are three ways to resolve this:

1. pass -v Eval.GdbSyncChunks=1 in the command line. This forces the VM to sync
debug info synchronously with gdb.

2. call HPHP::g_context.m_node.m_p->syncGdbState() from the gdb CLI. This
forces a manual sync of all outstanding symbols to gdb.

3. if the program has hit a seg fault (or another signal), press continue on
the CLI. The HHVM signal handler will sync outstanding DWARF symbols to gdb,
and a subsequent 'bt' should show all symbols.
