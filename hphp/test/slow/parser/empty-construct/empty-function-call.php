<?php

function foo() {
  return true;
}

<<__EntryPoint>>
function main_empty_function_call() {
error_reporting(-1);

var_dump(empty(foo()));
}
