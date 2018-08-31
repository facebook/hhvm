<?php

class Asd {
  static $y;
}

function foo() {
  $y =& Asd::$y;
  $y = 2;
}


<<__EntryPoint>>
function main_public_static_props_008() {
foo();
var_dump(Asd::$y);
}
