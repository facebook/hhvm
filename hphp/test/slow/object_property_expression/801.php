<?php

class X {
  function ref(&$ref) {
 $ref = 1;
 }
  function bar() {
    $this->ref($this->priv);
  }
}
;
class Y extends X {
 private $priv;
 }
class Z extends Y {
}
$z = new Z;
$z->bar();
var_dump($z);
$y = new Y;
$y->bar();
var_dump($y);
