<?hh
<<__EntryPoint>> function main(): void {
  echo "Basic test of POSIX uname function\n";

  $uname = posix_uname();
  unset($uname['domainname']);
  print_r($uname);
  echo "===DONE====";
}
