<?php
function f($a) {
  $a["four"] = 4;
  return $a;
}
$a = array(1=>1, 2=>2, 3=>3);
unset($a[1]);
unset($a[2]);
unset($a[3]);
var_dump(f($a));
