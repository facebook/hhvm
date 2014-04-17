<?php

function foo() {
  $x = new stdClass;
  $x->foo = 12;
  extract($x);
  var_dump(get_defined_vars());
}

foo();
