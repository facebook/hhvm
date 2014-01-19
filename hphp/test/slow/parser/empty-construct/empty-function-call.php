<?php
error_reporting(-1);

function foo() {
  return true;
}

var_dump(empty(foo()));
