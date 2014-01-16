<?php

$a = 4;
$b = 'i am a string';

function fn($c) { return $c + 5; }

$lam = function ($c) use ($a, $b) {
  return $a + $c;
};

class C {
  public static $i;
  public $j;

  public function meth($c) { return $c + 3; }
}

$refl = new ReflectionFunction('fn');
var_dump($refl->getStaticVariables());

$refl = new ReflectionFunction($lam);
var_dump($refl->getStaticVariables());

$refl = new ReflectionMethod('C', 'meth');
var_dump($refl->getStaticVariables());
