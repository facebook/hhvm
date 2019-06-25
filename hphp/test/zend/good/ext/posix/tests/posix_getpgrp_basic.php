<?hh
  echo "Basic test of POSIX getpgrp function\n";

  $pgrp = posix_getpgrp();

  var_dump($pgrp);
<<__EntryPoint>> function main(): void {
echo "===DONE====";
}
