<?hh
class IterableClass implements \HH\Iterable {
  use StrictIterable;
  public function getIterator() :mixed{
    return new ArrayIterator(vec[1, 2, 3]);
  }
}

class KeyedIterableClass implements \HH\KeyedIterable {
  use StrictKeyedIterable;
  public function getIterator() :mixed{
    return new ArrayIterator(vec[1, 2, 3]);
  }
}

function dump($iterable) :mixed{
  echo get_class($iterable) . "\n";
  foreach ($iterable as $k => $v) {
    echo "$k => $v\n";
  }
}

function test($lhs, $rhs) :mixed{
  echo get_class($lhs) . " concat ";
  echo (is_array($rhs) ? "array" : get_class($rhs)) . " = ";
  dump($lhs->concat($rhs));
}

function main() :mixed{
  $concatable = Vector {
    Vector {1, 2, 3},
    ImmVector {1, 2, 3},
    Map {'a' => 1, 'b' => 2, 'c' => 3},
    ImmMap {'a' => 1, 'b' => 2, 'c' => 3},
    Set {1, 2, 3},
    ImmSet {1, 2, 3},
    Pair {1, 2},
    (Vector {1, 2, 3})->lazy(),
    (Set {1, 2, 3})->lazy(),
    new IterableClass,
    new KeyedIterableClass,
  };

  foreach ($concatable as $lhs) {
    test ($lhs, vec[1, 2, 3]);
    foreach ($concatable as $rhs) {
      test($lhs, $rhs);
    }
  }
}

<<__EntryPoint>>
function main_concat() :mixed{
main();
}
