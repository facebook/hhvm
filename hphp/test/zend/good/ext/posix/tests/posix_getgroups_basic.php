<?hh
<<__EntryPoint>> function main(): void {
  echo "Basic test of POSIX getgroups\n";

  $groups = posix_getgroups();

  if (!is_array($groups)) {
      echo "TEST FAILED - array result expected\n";
  } else {
      echo "TEST PASSED\n";
  }
  echo "===DONE===\n";
}
