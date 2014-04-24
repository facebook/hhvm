<?hh
function dump($iterable) {
  echo get_class($iterable) . "\n";
  foreach ($iterable as $k => $v) {
    echo "$k => $v\n";
  }
}
function dump_set($iterable) {
  echo get_class($iterable) . "\n";
  foreach ($iterable as $v) {
    echo "$v\n";
  }
}
function main() {
  $x = Vector {1, 2, 3, 4, 5};
  dump($x->slice(1, 2));
  dump($x->slice(4, 10));
  dump($x->slice(10, 2));
  $x = ImmVector {1, 2, 3, 4, 5};
  dump($x->slice(1, 2));
  dump($x->slice(4, 10));
  dump($x->slice(10, 2));
  $x = Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5};
  dump($x->slice(1, 2));
  dump($x->slice(4, 10));
  dump($x->slice(10, 2));
  $x = ImmMap {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5};
  dump($x->slice(1, 2));
  dump($x->slice(4, 10));
  dump($x->slice(10, 2));
  $x = Set {1, 2, 3, 4, 5};
  dump_set($x->slice(1, 2));
  dump_set($x->slice(4, 10));
  dump_set($x->slice(10, 2));
  $x = ImmSet {1, 2, 3, 4, 5};
  dump_set($x->slice(1, 2));
  dump_set($x->slice(4, 10));
  dump_set($x->slice(10, 2));
  $x = Pair {1, 2};
  dump($x->slice(1, 1));
  dump($x->slice(1, 10));
  dump($x->slice(10, 2));
  $x = (Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5})->lazy();
  dump($x->slice(1, 2));
  dump($x->slice(4, 10));
  dump($x->slice(10, 2));
  $x = (Map {'a' => 1, 'b' => 2, 'c' => 3, 'd' => 4, 'e' => 5})->lazy()
         ->map($x ==> $x);
  dump($x->slice(1, 2));
  dump($x->slice(4, 10));
  dump($x->slice(10, 2));
  $x = (Set {1, 2, 3, 4, 5})->lazy();
  dump_set($x->slice(1, 2));
  dump_set($x->slice(4, 10));
  dump_set($x->slice(10, 2));
  $x = (Set {1, 2, 3, 4, 5})->lazy()->filter($x ==> true);
  dump_set($x->slice(1, 2));
  dump_set($x->slice(4, 10));
  dump_set($x->slice(10, 2));
}
main();
