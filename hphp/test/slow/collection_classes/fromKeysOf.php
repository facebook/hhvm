<?hh

function show_elems($fs) {
  echo '---- ', get_class($fs), ' ----', "\n";
  foreach ($fs as $e) {
    var_dump($e);
  }
  echo "----\n";
};

function set_from_keys() {
  echo "\nSet::fromKeysOf...\n";
  show_elems(Set::fromKeysOf(Vector {1, 2, 3}));
  show_elems(Set::fromKeysOf(['a', 'b', 'c']));
  show_elems(Set::fromKeysOf(Map {'a' => 1, 'b' => 2}));
  show_elems(Set::fromKeysOf(['a' => 1, 'b' => 2]));
  show_elems(Set::fromKeysOf(Set {4, 5, 6}));
}

function vector_from_keys() {
  echo "\nVector::fromKeysOf...\n";
  show_elems(Vector::fromKeysOf(Vector {1, 2, 3}));
  show_elems(Vector::fromKeysOf(['a', 'b', 'c']));
  show_elems(Vector::fromKeysOf(Map {'a' => 1, 'b' => 2}));
  show_elems(Vector::fromKeysOf(['a' => 1, 'b' => 2]));
  show_elems(Vector::fromKeysOf(Set {4, 5, 6}));
}

function main() {
  set_from_keys();
  vector_from_keys();
}
main();
