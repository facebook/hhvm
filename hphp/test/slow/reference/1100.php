<?php

function foo($a, &$b, $c) {
  $a+=1;
  $b+=2;
  $c+=3;
  var_dump($a,$b,$c);
}
function bar(&$a, &$b, &$c) {
  $a+=1;
  $b+=2;
  $c+=3;
  var_dump($a,$b,$c);
}
function test($fn, $arg) {
  $fn($arg, $arg, $arg);
  var_dump($arg);
}
test('foo', 1);
test('bar', 1);
