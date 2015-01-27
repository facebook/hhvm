<?php

class Asd {
  static $y;
}

function foo() {
  $y =& Asd::$y;
  $y = 2;
}

foo();
var_dump(Asd::$y);
