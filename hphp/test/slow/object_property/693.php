<?php

$a = array();
 $a[] = 1;
$o = (object)$a;
var_dump($o);
$s = serialize($o);
$o2 = unserialize($s);
