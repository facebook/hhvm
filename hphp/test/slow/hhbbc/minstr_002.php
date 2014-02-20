<?php

function foo() {
  $x->foo = "heh";
  return $x;
}

function bar() {
  var_dump(foo());
}

bar();


