<?php

declare(ticks=1);
function foo() {
  echo 'a';
  exit(1);
}
function bar() {
}
 pcntl_signal(SIGUSR1,  'foo');
$pid = posix_getpid();
posix_kill($pid, SIGUSR1);
bar();
 /* check for received signal upon function entry/exit */ for ($i = 0;
 $i < 2;
 $i++) {
  echo 'a';
}
