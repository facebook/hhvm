<?hh
  echo "Basic test of POSIX getpwuid\n";


  $pwuid = posix_getpwuid(posix_getuid());

  print_r($pwuid);
<<__EntryPoint>> function main(): void {
echo "===DONE====";
}
