<?php

$arr = new ArrayObject(array(1, 2));

$iter1 = new ArrayIterator($arr);
var_dump($iter1->getArrayCopy());
$iter2 = new ArrayIterator($iter1);
var_dump($iter2->getArrayCopy());
