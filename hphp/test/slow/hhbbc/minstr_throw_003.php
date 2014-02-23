<?php

function err($x) { throw new Exception(); }
set_error_handler('err');
function foo() {
  try {
    $x[][][]->foo = 2;
  } catch (Exception $e) {
    var_dump(is_array($x));
    var_dump($x);
  }
}
foo();
