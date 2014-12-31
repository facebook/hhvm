<?php

$in = [1,2,3];
$obj = new ArrayObject($in);
$out = $obj->exchangearray([4,5,6]);
var_dump($out);
$out[1] = 'herp';
var_dump($in);
