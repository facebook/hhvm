<?php


class MyIterator extends ArrayIterator {}


<<__EntryPoint>>
function main_array_object() {
$a = new ArrayObject();
$a->foo = "hello";
$a->setIteratorClass('MyIterator');
var_dump($a->foo);
var_dump($a->getIteratorClass());
$ser = unserialize(serialize($a));
var_dump($ser->foo);
var_dump($ser->getIteratorClass());
}
