<?php


<<__EntryPoint>>
function main_offset_exists() {
$arrayobj = new ArrayObject(array('zero', 'one', 'example'=>'e.g.', 'foo'=>null));
var_dump($arrayobj->offsetExists(1));
var_dump($arrayobj->offsetExists('example'));
var_dump($arrayobj->offsetExists('notfound'));
var_dump($arrayobj->offsetExists('foo'));
}
