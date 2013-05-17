<?php

function main() {

  $a = array('1' => '2', 'hello' => 'world', '' => 'empty');
  $b = null;

  var_dump(hphp_array_idx('1', $a, 3));
  var_dump(hphp_array_idx('0', $a, 3));
  var_dump(hphp_array_idx(1, $a, 3));
  var_dump(hphp_array_idx(0, $a, 3));
  var_dump(hphp_array_idx(1.01, $a, 3));
  var_dump(hphp_array_idx(true, $a, 3));
  var_dump(hphp_array_idx(false, $a, 3));

  var_dump(hphp_array_idx('hello', $a, 3));
  var_dump(hphp_array_idx('world', $a, 3));
  var_dump(hphp_array_idx('', $a, 3));
  var_dump(hphp_array_idx(null, $a, 3));

  // should fatal
  var_dump(hphp_array_idx('not_reached', $b, 3));
}

main();
