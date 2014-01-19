<?php

$a = array(1);
$b = array(2);
$arr = array($b, $a);
print $arr[0][0];
asort($arr, SORT_REGULAR, true);
 print $arr[0][0];
