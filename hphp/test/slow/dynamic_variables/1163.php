<?php

function test($a) {
  $b = 5;
  global $$a;
  var_dump($b);
}
test('b');
