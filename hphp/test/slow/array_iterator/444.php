<?php


$a = array(1, 2, 3);
foreach ($a as $k1 => &$v1) { $v1 += $k1; }
var_dump($a);
$a = array(1, 2, 3);
for ($o = new MutableArrayIterator($a); $o->valid(); $o->next()) {
  $k2 = $o->key();
  $v2 = &$o->currentRef();
  $v2 += $k2;
}
var_dump($a);
