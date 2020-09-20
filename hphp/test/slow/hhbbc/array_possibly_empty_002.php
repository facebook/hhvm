<?hh


class someclass {
  static public function yo() {
    return $_ENV['hey'] ?? false;
  }
}

function asd() { return mt_rand() ? 'a' : 2; }
function array_of_one() { return varray['ZZZZ']; }
function main() {
  $time = asd();
  $items = darray[];
  if (someclass::yo()) {
    $items = array_of_one();
  }
  $items[1] = $time;
  return $items;
}

<<__EntryPoint>>
function main_array_possibly_empty_002() {
main();
}
