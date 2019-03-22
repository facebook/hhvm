<?php

class Ref {
  public function __construct(public $val) {}
}

function new_closure_gen() {
  $ref = new Ref(0);
  return function() use ($ref) {
    yield ++$ref->val;
  };
}

$closure1 = new_closure_gen();
$closure2 = new_closure_gen();

$gen1 = $closure1();
$gen2 = $closure1();
$gen3 = $closure2();

foreach (array($gen1, $gen2, $gen3) as $gen) {
    foreach ($gen as $val) {
        var_dump($val);
    }
}

