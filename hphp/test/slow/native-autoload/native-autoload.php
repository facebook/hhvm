<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_native_autoload(): void {
  test_fn(new TestClass());

  $is_native = HH\autoload_is_native() ? 'true' : 'false';
  print "Native Autoload: $is_native\n";
  print "Paths:\n";
  try {
    $autoload_paths = HH\autoload_get_paths();
    \sort(inout $autoload_paths);
    foreach ($autoload_paths as $path) {
      print "  $path\n";
    }
  } catch (Exception $_) {
    print "FAIL!\n";
  }
}
