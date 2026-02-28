<?hh

function foo() :mixed{
  echo "a\n";
  exit(1);
}

function main() :mixed{
  var_dump(pcntl_signal(SIGUSR1, foo<>));
  $pid = posix_getpid();
  posix_kill($pid, SIGUSR1);

  // Wait for signal to arrive.
  while (true) continue;
}


<<__EntryPoint>>
function main_1563() :mixed{
main();
}
