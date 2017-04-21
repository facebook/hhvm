<?php

function test($fun) {
  $acc = 0;
  $callback = function (&$value, $key, $extra = 1) use(&$acc) {
    $value += $extra;
    $acc += $value;
    var_dump($key);
  };

  $fruit = array('lemons' => 2);
  $fun($fruit, $callback);
  var_dump($fruit['lemons']);
  var_dump($acc);
}

test('array_walk');
