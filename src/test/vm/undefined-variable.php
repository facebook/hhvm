<?php
function foo() {
  $a = $b;
}
foo();

function baz($x) {}
function bar() {
  baz($a);
}
bar();
