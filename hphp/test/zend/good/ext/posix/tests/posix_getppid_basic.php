<?hh
<<__EntryPoint>> function main(): void {
  echo "Basic test of POSIX getppid function\n";

  $ppid = posix_getppid();

  var_dump($ppid);
  echo "===DONE====";
}
