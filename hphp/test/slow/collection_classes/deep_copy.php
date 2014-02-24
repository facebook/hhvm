<?hh

// Test the deep copy mechanism that is triggered
// when collection literals are used as initializers
// for class properties.

class A {

  public $fv  = FixedVector {FixedVector {1, 2}, 3};
  public $v   = Vector {Vector {1, 2}, 3};
  public $s   = Set {1, 2, 3};
  public $m   = Map {0 => Map{0 => 1}, 1 => 2, 2 => 3};
  public $sm  = StableMap {0 => StableMap{0 => 1}, 1 => 2, 2 => 3};
  public $p   = Pair {Pair{1, 2}, 3};

}

function main() {
  $a1 = new A();
  $a2 = new A();

  // Check that every instance gets their own copy
  // of the collection literal.

  echo "\nVector...\n";
  $a1->v[0][] = Vector {1};
  print_r($a1->v);
  print_r($a2->v);

  echo "\nSet...\n";
  $a1->s->add(4);
  var_dump($a1->s == $a2->s);

  echo "\nMap...\n";
  $a1->m[3] = 4;
  var_dump($a1->m == $a2->m);

  echo "\nStableMap...\n";
  $a1->sm[3] = 4;
  print_r($a1->sm);
  print_r($a2->sm);

  // Pair and FixedVector are immutable, so
  // we can't do a similar test for them.

  echo "\nPair...\n";
  print_r($a1->p);
  print_r($a2->p);

  echo "\nFixedVector...\n";
  print_r($a1->fv);
  print_r($a2->fv);
}

main();
