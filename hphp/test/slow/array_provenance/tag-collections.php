<?hh

function short_provenance($x) {
  if (is_object($x)) return '<object>';
  if ($x === null)   return '<null>';
  $parts = explode('/', HH\get_provenance($x));
  return $parts[count($parts) - 1];
}

function test($cow_collections_array, $mutate_collections) {
  print("\n==============================================================\n");
  print("test(cow_collections_array=$cow_collections_array, mutate_collections=$mutate_collections):\n");

  $x = varray[];
  $x[] = varray[];
  $x[] = new Vector();
  $x[1]->append($x);
  if ($cow_collections_array) {
    $x[1]->append(vec($x[1]));
  }

  $flags = $mutate_collections ? TAG_PROVENANCE_HERE_MUTATE_COLLECTIONS : 0;
  $y = HH\tag_provenance_here($x, $flags);
  print('$y          => '.short_provenance($y)."\n");
  print('$y[0]       => '.short_provenance($y[0])."\n");
  print('$y[1]       => '.short_provenance($y[1])."\n");
  print('$y[1][0]    => '.short_provenance($y[1][0])."\n");
  print('$y[1][1]    => '.short_provenance(idx($y[1], 1))."\n");
  print('$y[1][0][0] => '.short_provenance($y[1][0][0])."\n");
  print('$y[1][0][1] => '.short_provenance($y[1][0][1])."\n");
}

function test_in_place($mutate_collections) {
  print("\n==============================================================\n");
  print("test_in_place(mutate_collections=$mutate_collections):\n");

  $x = varray[Vector{}, Vector{}, varray[]];
  $x[0]->append($x);
  $x[1]->append($x);

  $flags = $mutate_collections ? TAG_PROVENANCE_HERE_MUTATE_COLLECTIONS : 0;
  $y = HH\tag_provenance_here($x, $flags);
  print('$y          => '.short_provenance($y)."\n");
  print('$y[0]       => '.short_provenance($y[0])."\n");
  print('$y[1]       => '.short_provenance($y[1])."\n");
  print('$y[2]       => '.short_provenance($y[2])."\n");
  print('$y[0][0]    => '.short_provenance($y[0][0])."\n");
  print('$y[1][0]    => '.short_provenance($y[1][0])."\n");
  print('$y[0][0][0] => '.short_provenance($y[0][0][0])."\n");
  print('$y[0][0][1] => '.short_provenance($y[0][0][1])."\n");
  print('$y[0][0][2] => '.short_provenance($y[0][0][2])."\n");
  print('$y[1][0][0] => '.short_provenance($y[1][0][0])."\n");
  print('$y[1][0][1] => '.short_provenance($y[1][0][1])."\n");
  print('$y[1][0][2] => '.short_provenance($y[1][0][2])."\n");
}

<<__EntryPoint>>
function main() {
  test(0, 0);
  test(0, 1);
  test(1, 0);
  test(1, 1);

  test_in_place(0);
  test_in_place(1);
}
