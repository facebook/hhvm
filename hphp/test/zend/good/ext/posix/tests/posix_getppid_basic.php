<?hh
  echo "Basic test of POSIX getppid function\n";

  $ppid = posix_getppid();

  var_dump($ppid);
<<__EntryPoint>> function main(): void {
echo "===DONE====";
}
