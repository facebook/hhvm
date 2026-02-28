<?hh

function print_cover_maps(dict<string, vec<int>> $map) :mixed{
  print("\nPrinting coverage:\n");
  foreach ($map as $path => $lines) {
    sort(inout $lines);
    print("  $path: ".json_encode($lines)."\n");
  }
}

<<__EntryPoint>>
function main() :mixed{
  HH\enable_per_file_coverage(keyset[
    __DIR__.'/interface-definition.inc',
  ]);
  var_dump(17 is ITest);
  print_cover_maps(HH\get_all_coverage_data());
}
