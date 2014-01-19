<?php

function bar() {
}
function foo() {
  $foo = bar();
  unset($foo);
}
foo();
