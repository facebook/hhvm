<?php
function foo() {
}
function goo(&$p) {
}
function bar() {
}

<<__EntryPoint>>
function main_1187() {
$goo = 'goo';
goo(&foo());
$goo(&foo());
bar(foo());
}
