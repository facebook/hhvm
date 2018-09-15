<?php


<<__EntryPoint>>
function main_1544() {
$a = array(array());
$a[0][0] = &$a[0];
var_dump(serialize($a));
}
