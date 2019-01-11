<?php

function foo() {
  $x = new stdClass;
  $x->foo = 12;
  extract(&$x);
  var_dump(get_defined_vars());
}


<<__EntryPoint>>
function main_extract_non_array() {
foo();
}
