<?hh
class C implements \HH\Iterable {
  use StrictIterable;
  public function getIterator() {
    return new ArrayIterator(array(1, 2, 3));
  }
}
class D implements \HH\Iterable {
  use StrictKeyedIterable;
  public function getIterator() {
    return new ArrayIterator(array(1, 2, 3));
  }
}
function dump($iterable) {
  echo get_class($iterable) . "\n";
  foreach ($iterable as $k => $v) {
    echo "$k => $v\n";
  }
}
function main() {
  $x = Vector {1, 2, 3};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = ImmVector {1, 2, 3};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = Map {'a' => 1, 'b' => 2, 'c' => 3};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = ImmMap {'a' => 1, 'b' => 2, 'c' => 3};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = Set {1, 2, 3};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = ImmSet {1, 2, 3};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = Pair {1, 2};
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = Set {1, 2, 3};
  dump($x->lazy()->concat(array(4, 5))->toImmVector());
  dump($x->lazy()->concat(Vector {4, 5})->toImmVector());
  $x = Vector {1, 2, 3};
  dump($x->lazy()->concat(array(4, 5))->toImmVector());
  dump($x->lazy()->concat(Vector {4, 5})->toImmVector());
  $x = new C;
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
  $x = new D;
  dump($x->concat(array(4, 5)));
  dump($x->concat(Vector {4, 5}));
}
main();
