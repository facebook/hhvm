<?php

function err($x) { throw new Exception(); }
function foo() {
  try {
    $x[][][]->foo = 2;
  } catch (Exception $e) {
    var_dump(is_array($x));
    var_dump($x);
  }
}

<<__EntryPoint>>
function main_minstr_throw_003() {
set_error_handler('err');
foo();
}
