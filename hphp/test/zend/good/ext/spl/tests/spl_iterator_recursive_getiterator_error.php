<?php
$i = new ArrayIterator(array(1,1,1,1,1));
$iii = new IteratorIterator($i);
p($iii);
function p ($i) {
  foreach ($i as &$value) {}
}
?>