<?php 

$arr = array(array(1,2));
$arrOb = new ArrayObject($arr);

$recArrIt = new RecursiveArrayIterator($arrOb->getIterator());

$recItIt = new RecursiveIteratorIterator($recArrIt);

foreach ($recItIt as &$val) echo "$val\n";

?>