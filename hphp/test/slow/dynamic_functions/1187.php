<?php

$a = 'test';
if ($a) {
  function bar() {
}
}
 else {
  function bar() {
}
}
function foo() {
}
function goo(&$p) {
}
$goo = 'goo';
goo(foo());
$goo(foo());
bar(foo());
