<?php


<<__EntryPoint>>
function main_offset_unset() {
$arrayobj = new ArrayObject(array(0=>'zero',2=>'two'));
$arrayobj->offsetUnset(2);
var_dump($arrayobj);
}
