<?php


<<__EntryPoint>>
function main_setm_inner_array() {
$obj = new ArrayObject(array('key' => array()));
$obj['key']['other_key'] = 'other_val';

var_dump($obj['key']);
}
