
<h2>Useful gdb Commands</h2>

  info threads
  thread apply [threadno] [all] args
  set follow-fork-mode child
  set detach-on-fork off
  handle SIGPIPE nostop noprint pass
  dump binary memory [filename] [start_addr] [end_addr]

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
