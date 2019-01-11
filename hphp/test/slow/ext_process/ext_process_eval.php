<?php


<<__EntryPoint>>
function main_ext_process_eval() {
$pid = pcntl_fork();
if ($pid == 0) {
  $a = 1;
  eval('$a = 42;');
  print $a . "\n";
  exit(5);
}
pcntl_wait(&$status);
$a = 2;
eval('$a = 53;');
print $a . "\n";
}
