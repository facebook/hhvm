<?php
function fix(&$v, $k) {
 $v *= 2;
 }


<<__EntryPoint>>
function main_1771() {
$a = array('foo'=>array('bar'=>1));
array_walk_recursive($a, 'fix');
var_dump($a['foo']);
}
