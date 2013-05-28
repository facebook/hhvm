<?php

function handler() {
 var_dump(__METHOD__);
 return true;
 }
set_error_handler('handler');

function foo(array &$a) {
 var_dump($a);
 }

function test($a) {
  foo($a);
}

test("hello");
test(array(1,2,3));
test(array());
test("hello");

