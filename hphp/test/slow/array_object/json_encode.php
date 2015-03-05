<?php


$arr = [];
$ao = new ArrayObject($arr, ArrayObject::STD_PROP_LIST);
var_dump($ao);
$ao->var1 = 'var1';
var_dump((array) $ao);
echo json_encode($ao) . "\n";
