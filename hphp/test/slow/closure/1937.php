<?php

function test($a, $b) {
  return array_map(function (array $x) use ($b) {
      var_dump($x,$b);
    }
, $a);
}

<<__EntryPoint>>
function main_1937() {
test(array(array(1), array(2)), 5);
}
