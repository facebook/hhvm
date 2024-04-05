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
    __DIR__.'/generic-class.inc',
  ]);
  cover_generic_class();
  print_cover_maps(HH\get_all_coverage_data());
}
