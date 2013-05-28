<?php

$v = 42;
$o1 = new stdclass;
$o2 = new stdclass;
$o1->p = &$v;
$o2->p = &$v;
$arr1 = array($o1, $o2);
apc_store('foo', $arr1);
$arr2 = apc_fetch('foo');
var_dump($arr1);
var_dump($arr2);
