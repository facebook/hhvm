<?php
$ao = new ArrayObject(array("a" => 1, "b" => 2, "c" => 3));
var_dump($ao->getIterator());
var_dump((array)$ao->getIterator());
