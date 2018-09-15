<?php


<<__EntryPoint>>
function main_1126() {
$a = array(1,2, '' => 'foo');
unset($a[null]);
var_dump($a);
}
