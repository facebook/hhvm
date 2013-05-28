<?php

function test1() {
  var_dump(__METHOD__);
}
function test2() {
  var_dump(__METHOD__);
}
function test($test) {
  test1();
  TeSt1();
  $test();
  $test = strtolower($test);
  $test(1,2,3);
}
test('Test1');
fb_rename_function('tEst1', 'fiz');
fb_rename_function('test2', 'Test1');
test('teSt1');
