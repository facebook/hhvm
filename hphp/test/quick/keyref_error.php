<?php

$a = array(3 => 5);
foreach ($a as &$b => &$c) {
  $b += 1;
  $c -= 1;
}
var_export($a);
