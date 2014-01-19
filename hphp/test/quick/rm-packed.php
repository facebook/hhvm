<?php
function main($a, $i) {
  unset($a[$i]);
  $a[] = 'foo';
  return $a;
}
var_dump(main(array('a', 'b'), 1));
