<?php

class X {
  public $x = 1;
  }
class Y {
  public $x = array();
}
function test() {
  $x = (object)new Y;
  var_dump($x->x);
  $x = new X;
  var_dump($x->x);
}
test();
