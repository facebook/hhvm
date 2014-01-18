<?php

function instance_of_test(array $x) {
  var_dump($x instanceof \HH\Traversable);
  var_dump($x instanceof Traversable); // false without
                                       // EnableHipHopSyntax or <?hh
  var_dump($x instanceof XHPChild);
  var_dump($x instanceof Indexish);
  var_dump($x instanceof KeyedTraversable);
}

function type_hint_test(KeyedTraversable $x) {
  var_dump(is_array($x));
}

instance_of_test(array(1,2,3));
type_hint_test(array(1,2,3));
