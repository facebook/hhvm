<?php

class A {
  public $foo;
  public $bar;
  function q($a) {
    echo $a;
    $this->foo = 9;
    $this->bar = '3';
    return $this;
  }
}
$a = new A();
var_dump($a->q('1')->foo + $a->q('2')->bar);
var_dump($a->q('1')->foo - $a->q('2')->bar);
var_dump($a->q('1')->foo / $a->q('2')->bar);
var_dump($a->q('1')->foo * $a->q('2')->bar);
var_dump($a->q('1')->foo % $a->q('2')->bar);
var_dump($a->q('1')->foo << $a->q('2')->bar);
var_dump($a->q('1')->foo >> $a->q('2')->bar);
var_dump($a->q('1')->foo && $a->q('2')->bar);
var_dump($a->q('1')->foo || $a->q('2')->bar);
var_dump($a->q('1')->foo and $a->q('2')->bar);
var_dump($a->q('1')->foo or $a->q('2')->bar);
var_dump($a->q('1')->foo xor $a->q('2')->bar);
var_dump($a->q('1')->foo . $a->q('2')->bar);
var_dump($a->q('1')->foo & $a->q('2')->bar);
var_dump($a->q('1')->foo | $a->q('2')->bar);
var_dump($a->q('1')->foo ^ $a->q('2')->bar);
var_dump($a->q('1')->foo == $a->q('2')->bar);
var_dump($a->q('1')->foo === $a->q('2')->bar);
var_dump($a->q('1')->foo != $a->q('2')->bar);
var_dump($a->q('1')->foo !== $a->q('2')->bar);
var_dump($a->q('1')->foo > $a->q('2')->bar);
var_dump($a->q('1')->foo >= $a->q('2')->bar);
var_dump($a->q('1')->foo < $a->q('2')->bar);
var_dump($a->q('1')->foo <= $a->q('2')->bar);
