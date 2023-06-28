<?hh

function handler($signo) :mixed{
  echo "handler received signal $signo, will do SIG_IGN later\n";
  pcntl_signal($signo, SIG_IGN);
}
<<__EntryPoint>> function main(): void {
pcntl_signal(SIGINT, handler<>);

$pid = posix_getpid();
for ($i = 0; $i < 3; ++$i) {
  posix_kill($pid, SIGINT);
  pcntl_signal_dispatch();
}

echo "resetting to SIG_DFL\n";
pcntl_signal(SIGINT, SIG_DFL);

posix_kill($pid, SIGINT);
pcntl_signal_dispatch();

echo "should not be printed\n";
}
