<?php
$i = PHP_INT_MAX;
$arr = array();
$arr[(string)$i] = 1;
$i = -$i;
$arr[(string)$i] = 1;
--$i;
$arr[(string)$i] = 1;
foreach ($arr as $k => $v) {
  var_dump($k);
}
