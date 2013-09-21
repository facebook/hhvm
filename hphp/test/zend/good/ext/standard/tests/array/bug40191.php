<?php

$arrObj = new ArrayObject();
$arrObj->append('foo');
$arrObj->append('bar');
$arrObj->append('foo');

$arr = array_unique($arrObj);
var_dump($arr);

echo "Done\n";
?>