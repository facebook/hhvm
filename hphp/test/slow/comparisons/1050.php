<?php

class c {
  public $x = 0;
}

<<__EntryPoint>>
function main_1050() {
$x = new c;
$x->x = 1;
$y = new c;
var_dump($x > $y);
var_dump(array($x) == array($y));
}
