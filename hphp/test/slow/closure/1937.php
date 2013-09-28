<?php

function test($a, $b) {
  return array_map(function (array $x) use ($b) {
      var_dump($x,$b);
    }
, $a);
}
test(array(array(1), array(2)), 5);
