<?php
function test($v, $k) {
 }
function foo($a) {
 var_dump($a);
 }


<<__EntryPoint>>
function main_1772() {
$a = array(array('one'),array('one'));
array_walk_recursive($a, 'foo');
}
