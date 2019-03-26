<?php

function set_ref(&$ref, $val) {
  $ref = $val;
}

<<__EntryPoint>>
function main_1058() {
  $a = array(1, 'test');
  $b = $a;
  set_ref(&$b[0], 10);
  var_dump($a, $b);
}
