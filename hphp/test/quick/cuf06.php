<?php

set_error_handler('handler');

function handler($a, $b) {
  var_dump($a, $b);
}

function foo() {}

function test() {
  call_user_func_array('foo', array());
}

function main() {
  test();
  fb_rename_function('foo', 'bar');
  test();
}

main();
