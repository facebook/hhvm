<?hh
<<__EntryPoint>> function main(): void {
  $test_loc = null;
  $a = null;
  $e = keyset[];
  try {
    var_dump(main<> + 42); // this will throw
    $test_loc = 0;
    $a += $e;
  } catch (Exception $e) {
    // need to not assume $test_loc is int
    if ($test_loc is int) {
      echo "hello, yes, we are an int";
    } else {
      echo "not an int!";
    }
  }
}
