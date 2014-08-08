<?php

$array = array('' => 1, 1 => 2, 3 => 4);
$ArrayObject = new ArrayObject($array);
var_dump($ArrayObject);
$Iterator = $ArrayObject->getIterator();
var_dump(count($Iterator) === count($array));
var_dump(iterator_to_array($Iterator));

?>
