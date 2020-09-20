<?hh
<<__EntryPoint>> function main(): void {
  echo "Basic test of POSIX getpgrp function\n";

  $pgrp = posix_getpgrp();

  var_dump($pgrp);
  echo "===DONE====";
}
