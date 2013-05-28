<?php

function test($a, $b) {
  return $a . "\0" . $b . "\0" . $a . $b . $a . $b;
}
var_dump(json_encode(test('x', 'y')));
