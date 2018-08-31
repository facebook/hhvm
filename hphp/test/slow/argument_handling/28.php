<?php

function test($str) {
  return strlen($str);
}

<<__EntryPoint>>
function main_28() {
var_dump(strlen());
var_dump(test());

var_dump(strlen('test'));
var_dump(test('test'));

var_dump(strlen('test', 123));
var_dump(test('test', 123));
}
