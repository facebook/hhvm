<?hh

function handle_autoload($kind, $name) {
  print("In handle_autoload($kind, $name)\n");
  if ($kind === 'class' && $name === 'ITest') {
    require __DIR__.'/interface-definition.inc';
  }
}

function print_cover_maps(dict<string, vec<int>> $map) {
  print("\nPrinting coverage:\n");
  foreach ($map as $path => $lines) {
    sort(inout $lines);
    print("  $path: ".json_encode($lines)."\n");
  }
}

<<__EntryPoint>>
function main() {
  HH\autoload_set_paths(darray['failure' => handle_autoload<>], __DIR__);
  HH\enable_per_file_coverage(keyset[
    __DIR__.'/interface-definition.inc',
  ]);
  var_dump(17 is ITest);
  print_cover_maps(HH\get_all_coverage_data());
}
