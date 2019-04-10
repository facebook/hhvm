<?php

function handler() {
 var_dump(__METHOD__);
 return true;
 }

function foo(@array &$a) {
 var_dump($a);
 }

function test($a) {
  foo(&$a);
}


<<__EntryPoint>>
function main_ref_type_fail() {
set_error_handler('handler');

test("hello");
test(array(1,2,3));
test(array());
test("hello");
}
