<?php
$a = function ($add) {
  return $this->x + $add;
};

class FooBar {
  private $x = 3;
}

$fb = new FooBar();

// bindTo
$bound = $a->bindTo($fb, "FooBar");
$r1 = $bound(4);
var_dump($r1);

// call
$r2 = $a->call($fb, 4);
var_dump($r2);
