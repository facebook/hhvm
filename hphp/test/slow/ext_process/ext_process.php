<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x !== false, true); }

//////////////////////////////////////////////////////////////////////

$pid = pcntl_fork();
if ($pid == 0) {
  exit(123);
}
pcntl_wait($status);

$pri = pcntl_getpriority();
VERIFY($pri);
if ($pri < 15) {
  VS(pcntl_setpriority(15), true);
  VS(pcntl_getpriority(), 15);
} else {
  var_dump(true, true);
}

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_wait($status);
VS($status, 0x1200);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VS($status, 0x1200);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x80);
}
pcntl_waitpid(0, $status);
VS(pcntl_wexitstatus($status), 0x80);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VERIFY(pcntl_wifexited($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VERIFY(!pcntl_wifsignaled($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VERIFY(!pcntl_wifstopped($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VS(pcntl_wstopsig($status), 0x12);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VS(pcntl_wtermsig($status), 0);

