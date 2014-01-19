<?php

function test1() {
 echo "test1
";
 }
function test3() {
 echo "test3
";
 }
function baz($test1, $test2) {
  var_dump(function_exists("Test1"));
  var_dump(function_exists("tEst2"));
  var_dump(function_exists($test1));
  var_dump(function_exists($test2));
}
baz("teSt1", "test2");
fb_rename_function("test1", "test2");
baz("TEst1", "test2");
fb_rename_function("test3", "test1");
baz("test1", "test2");
test1();
test2();
