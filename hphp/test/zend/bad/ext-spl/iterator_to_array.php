<?php
$array=array('a','b');

$iterator = new ArrayIterator($array);

iterator_to_array();


iterator_to_array($iterator,'test','test');

iterator_to_array('test','test');

?>