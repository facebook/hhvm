<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_native_autoload_symlinked(): void {
  $is_native = HH\autoload_is_native() ? 'true' : 'false';

  print "Native Autoload: $is_native\n";

  test_fn();
}
