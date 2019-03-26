<?php

class Asd {
  static $y;
}

function bar(&$ref) {
  $ref = 2;
}


function foo() {
  bar(&Asd::$y);
}


<<__EntryPoint>>
function main_public_static_props_008() {
foo();
var_dump(Asd::$y);
}
