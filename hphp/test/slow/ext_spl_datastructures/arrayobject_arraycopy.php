<?php

$arr = new ArrayObject(array(1, 2));

$arrobj1 = new ArrayObject($arr);
var_dump($arrobj1->getArrayCopy());
$arrobj2 = new ArrayObject($arrobj1);
var_dump($arrobj2->getArrayCopy());
