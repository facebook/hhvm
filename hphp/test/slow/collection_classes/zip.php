<?hh

class IterableClass implements \HH\Iterable {
  use StrictIterable;
  public function getIterator() :mixed{
    return new ArrayIterator(vec[11, 22, 33]);
  }
}

class KeyedIterableClass implements \HH\KeyedIterable {
  use StrictKeyedIterable;
  public function getIterator() :mixed{
    return new ArrayIterator(vec[44, 55, 66]);
  }
}

function test($name, $a, $b) :mixed{
  echo "---- $name\n";
  $got = $a->zip($b);
  echo get_class($got) . "\n";
  foreach ($got as $k => $p) {
    echo "$k => ";
    if ($p is Pair) {
      echo "Pair {{$p[0]} => {$p[1]}}\n";
    } else {
      echo "ERROR\n";
      var_dump($p);
    }
  }
}

function main() :mixed{
  $zippable = dict[
    'empty Vector'    => Vector    {},
    'short Vector'    => Vector    {1, 2},
    'long  Vector'    => Vector    {3, 4, 5, 6},
    'empty ImmVector' => ImmVector {},
    'short ImmVector' => ImmVector {9, 8},
    'long  ImmVector' => ImmVector {7, 6, 5, 4},

    'empty Map'    => Map    {},
    'short Map'    => Map    {'a' => 'A', 'b' => 'B'},
    'long  Map'    => Map    {'c' => 'C', 'd' => 'D', 'e' => 'E', 'f' => 'F'},
    'empty ImmMap' => ImmMap {},
    'short ImmMap' => ImmMap {'u' => 'U', 'v' => 'V'},
    'long  ImmMap' => ImmMap {'w' => 'W', 'x' => 'X', 'y' => 'Y', 'z' => 'Z'},
  ];
  $additional_iterable = dict[
    'IterableClass'      => new IterableClass,
    'KeyedIterableClass' => new KeyedIterableClass,
  ];
  foreach ($zippable as $name => $container) {
    foreach ($zippable as $name2 => $container2) {
      test("$name zip with $name2", $container, $container2);
    }
    foreach ($additional_iterable as $name2 => $container2) {
      test("$name zip with $name2", $container, $container2);
    }
  }
}


<<__EntryPoint>>
function main_zip() :mixed{
main();
}
