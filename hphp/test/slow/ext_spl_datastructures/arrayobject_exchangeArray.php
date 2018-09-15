<?php


<<__EntryPoint>>
function main_arrayobject_exchange_array() {
$obj = new stdClass();
$obj->a = 4;
$ao = new ArrayObject($obj);
var_dump($ao->exchangeArray([]));
}
