<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function print_enabled_stats(): void {
  $native_autoload_enabled = HH\autoload_is_native() ? 'true' : 'false';
  print "Native Autoload: $native_autoload_enabled\n";
}

function sorted(Container<string> $xs): Container<string> {
  sort(inout $xs);
  return $xs;
}

<<__EntryPoint>>
function main_trusted_autoload(): void {
  test_fn(new TestClass());

  print_enabled_stats();

  print "Paths:\n";
  foreach (sorted(HH\autoload_get_paths()) as $path) {
    print "  $path:\n";
    print "    Types:\n";
    foreach (sorted(HH\autoload_path_to_types($path)) as $type) {
      $is_correct = $path === HH\autoload_type_to_path($type)
        ? 'PASS'
        : 'FAIL';
      print "      $type => $is_correct\n";
    }
    print "    Functions:\n";
    foreach (sorted(HH\autoload_path_to_functions($path)) as $function) {
      $is_correct = $path === HH\autoload_function_to_path($function)
        ? 'PASS'
        : 'FAIL';
      print "      $function => $is_correct\n";
    }
    print "    Constants:\n";
    foreach (sorted(HH\autoload_path_to_constants($path)) as $constant) {
      $is_correct = $path === HH\autoload_constant_to_path($constant)
        ? 'PASS'
        : 'FAIL';
      print "      $constant => $is_correct\n";
    }
    print "    TypeAliases:\n";
    foreach (sorted(HH\autoload_path_to_type_aliases($path)) as $type_alias) {
      $is_correct = $path === HH\autoload_type_alias_to_path($type_alias)
        ? 'PASS'
        : 'FAIL';
      print "      $type_alias => $is_correct\n";
    }
  }

  print "Overriding: ".HH\autoload_set_paths(
    darray['class' => darray[]],
    __DIR__,
  )."\n";

  print_enabled_stats();
}
