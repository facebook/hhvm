<?php

function fn($c) { return $c + 5; }

class C {
  public static $i;
  public $j;

  public function meth($c) { return $c + 3; }
}


<<__EntryPoint>>
function main_closure_get_static_vars() {
$a = 4;
$b = 'i am a string';

$lam = function ($c) use ($a, $b) {
  return $a + $c;
};

$refl = new ReflectionFunction('fn');
var_dump($refl->getStaticVariables());

$refl = new ReflectionFunction($lam);
var_dump($refl->getStaticVariables());

$refl = new ReflectionMethod('C', 'meth');
var_dump($refl->getStaticVariables());
}
