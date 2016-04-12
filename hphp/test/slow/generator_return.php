<?php

class A {
  public function Gen() {
    var_dump($this);
    yield 1; yield 2; yield 3;
    return 11;
  }

  public static function SGen() {
    var_dump(get_called_class());
    yield 4; yield 5; yield 6;
    return 22;
  }
}

function basicGen() {
  yield 7; yield 8;
  return 33;
}

function noReturnGen() {
  yield 9; yield 10;
}

$a = new A();
$g = $a->Gen();
foreach ($g as $num) { var_dump($num); }
var_dump($g->getReturn());

$g2 = A::SGen();
foreach ($g2 as $num) { var_dump($num); }
var_dump($g2->getReturn());

$g3 = basicGen();
foreach ($g3 as $num) { var_dump($num); }
var_dump($g3->getReturn());

$g4 = noReturnGen();
foreach ($g4 as $num) { var_dump($num); }
var_dump($g4->getReturn());
