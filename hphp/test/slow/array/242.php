<?php

function test($q, $a, $b, $c) {
  $x = array($a, 'foo'=> $a);
  if ($x) {
    var_dump(isset($x[0][1]), isset($x['foo'][1]));
    var_dump(isset($x[$b][1]), isset($x[$c][1]));
    var_dump(end($x[0]));
  }
}
test(5, array(0,1), 0, 'foo');
