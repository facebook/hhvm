<?php

function main() {

  $a = array('1' => '2', 'hello' => 'world', '' => 'empty');
  $b = null;

  var_dump(hphp_array_idx('1', $a, 3));
  var_dump(hphp_array_idx('0', $a, 4));
  var_dump(hphp_array_idx(1, $a, 5));
  var_dump(hphp_array_idx(0, $a, 6));
  var_dump(hphp_array_idx(1.01, $a, 7));
  var_dump(hphp_array_idx(true, $a, 8));
  var_dump(hphp_array_idx(false, $a, 9));

  var_dump(hphp_array_idx('hello', $a, 10));
  var_dump(hphp_array_idx('world', $a, 11));
  var_dump(hphp_array_idx('', $a, 12));
  var_dump(hphp_array_idx(null, $a, 13));

  // should fatal
  var_dump(hphp_array_idx('not_reached', $b, 14));
}

main();
