<?php
$ao = new ArrayObject(array('foo' => null));
var_dump($ao->offsetExists('foo'));

?>