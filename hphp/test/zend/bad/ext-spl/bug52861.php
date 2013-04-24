<?php
$arrayObject = new ArrayObject(array('foo' => array('bar' => array('baz' => 'boo'))));

unset($arrayObject['foo']['bar']['baz']);
print_r($arrayObject->getArrayCopy());
?>