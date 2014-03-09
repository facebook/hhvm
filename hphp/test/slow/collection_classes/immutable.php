<?hh
function main() {
  var_dump((Vector {1, 2})->immutable());
  var_dump((ImmVector {1, 2})->immutable());
  var_dump((Map {'a' => 1, 'a' => 2})->immutable());
  var_dump((ImmMap {'a' => 1, 'a' => 2})->immutable());
  var_dump((Set {1, 2})->immutable());
  var_dump((ImmSet {1, 2})->immutable());
  var_dump((Pair {1, 2})->immutable());
}
main();

