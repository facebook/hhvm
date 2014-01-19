<?php

class A {
  public $a = 'apple';
  public $b = 'banana';
}

$old = new A;
unset($old->a);
var_dump($old);

$new = new A;
unset($new->b);
var_dump($new);

foreach ($new as $property => $value) {
  $old->$property = $value;
}
var_dump($old->a);
var_dump($old->b);
