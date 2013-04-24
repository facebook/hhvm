<?php

// Bug #42850
$data = array ('key1' => 'val1', array('key2' => 'val2'));
function apply_dumb($item, $key) {}; 
var_dump($data);
array_walk_recursive($data, 'apply_dumb');
$data2 = $data;
$data2[0] = 'altered';
var_dump($data);
var_dump($data2);

// Bug #34982
function myfunc($data) {
    array_walk_recursive($data, 'apply_changed');
}
function apply_changed(&$input, $key) {
    $input = 'changed';
}
myfunc($data);
var_dump($data);
