<?php
function foo() {
}
function goo(&$p) {
}


<<__EntryPoint>>
function main_1187() {
$a = 'test';
if ($a) {
  function bar() {
}
}
 else {
  function bar() {
}
}
$goo = 'goo';
goo(&foo());
$goo(&foo());
bar(foo());
}
