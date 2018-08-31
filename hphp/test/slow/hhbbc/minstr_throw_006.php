<?php

function err($x) { throw new Exception(); }
function foo() {
  try {
    $x->a = 2;
  } catch (Exception $e) {
    echo "catch\n";
    restore_error_handler();
    var_dump(is_object($x));
    var_dump(is_null($x));
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_minstr_throw_006() {
set_error_handler('err');
foo();
}
