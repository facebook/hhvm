<?php

$a = array(1, 2, 3, 4, 5, 6);
while ($v = each($a)) {
 if ($v[1] < 4) $a[] = $v[1] + $v[1];
 }
var_dump($a);
$a = array(1, 2, 3, 4, 5, 6);
foreach ($a as $k => $v) {
 if ($v >= 4) $a = $v + $v;
 }
var_dump($a);
