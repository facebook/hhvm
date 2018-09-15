<?php

class FooBar {
  private $x = 3;
}

<<__EntryPoint>>
function main_call_and_bind_to() {
$a = function ($add) {
  return $this->x + $add;
};

$fb = new FooBar();

// bindTo
$bound = $a->bindTo($fb, "FooBar");
$r1 = $bound(4);
var_dump($r1);

// call
$r2 = $a->call($fb, 4);
var_dump($r2);
}
