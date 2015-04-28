<?hh

function main() {
  $input = Vector {"a", "b", "c", "d", "e"};

  var_dump(array_slice($input, 2));
  var_dump(array_slice($input, 2, null));
  var_dump(array_slice($input, -2, 1));
  var_dump(array_slice($input, 0, 3));
  var_dump(array_slice($input, 0, 5));

  // note the differences in the array keys
  var_dump(array_slice($input, 2, -1));
  var_dump(array_slice($input, 2, -1, true));

  var_dump(array_slice(Vector {"a", "b", "c"}, 1, 2, true));
  var_dump(array_slice(Vector {"a", "b", "c"}, 1, 2, false));

  $m = Map {"a" => "g", 0 => "a", 1 => "b", 2 => "c"};
  unset($m['a']);
  var_dump(array_slice($m, 1, 2, true));
  var_dump(array_slice($m, 1, 2, false));

  $m = Map {"a" => 123, 0 => "a", 1 => "b", 2 => "c"};
  unset($m['a']);
  var_dump(array_slice($m, 1, 2, true));
  var_dump(array_slice($m, 1, 2, false));

  var_dump(array_slice(ImmVector{123, "b", "c"}, 1, 2, true));
  var_dump(array_slice(ImmVector{123, "b", "c"}, 1, 2, false));

  var_dump(array_slice(Set {"a", "b", "c"}, 1, 2, true));
  var_dump(array_slice(ImmSet {"a", "b", "c"}, 1, 2, false));

  var_dump(array_slice(Map{12 => 1, 6 => 2}, 0, 2, true));
  var_dump(array_slice(Map{12 => 1, 6 => 2}, 0, 2, false));
}
main();
