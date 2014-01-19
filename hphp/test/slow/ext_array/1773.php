<?php

function xsort(&$a) {
  $b = null;
  $b->foo =& $a;
  var_dump(is_object($b));
  $b = false;
  $b[0] =& $a;
  uksort($a, function ($i, $j) use(&$b) {
      if ($b[0][$i] == $b[0][$j]) return 0;
      return $b[0][$i] < $b[0][$j] ? -1 : 1;
    }
);
}
function test($x) {
  $a = array(220,250,240,$x);
  xsort($a);
  var_dump($a);
}
test(230);
