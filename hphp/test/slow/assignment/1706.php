<?php

function e() {
 return 'hello';
 }
function foo() {
  $expected = e();
  $list_expected = "[$expected,$expected]";
  var_dump($expected, $list_expected);
}
foo();
