<?php

function instance_of_test(array $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x instanceof \HH\Traversable);
  var_dump($x instanceof Traversable); // false without
                                       // EnableHipHopSyntax or <?hh
  var_dump($x instanceof XHPChild);
  var_dump($x instanceof Indexish);
  var_dump($x instanceof Stringish);
  var_dump($x instanceof \HH\KeyedTraversable);
  var_dump($x instanceof KeyedTraversable); // false without
                                            // EnableHipHopSyntax or <?hh
}

function type_hint_traversable(\HH\KeyedTraversable $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump(is_array($x));
}

function type_hint_stringish(Stringish $x) {
  echo '==================== ', __FUNCTION__,' ====================', "\n";
  var_dump($x instanceof Stringish);
  var_dump(is_string($x));
}

class C {
  public function __toString() {
    return 'C';
  }
}

function main() {
  instance_of_test(array(1,2,3));
  type_hint_traversable(array(1,2,3));

  $c = new C();
  type_hint_stringish($c);
}
main();
