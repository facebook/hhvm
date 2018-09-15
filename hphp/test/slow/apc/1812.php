<?php


<<__EntryPoint>>
function main_1812() {
$a = array();
$a[] =& $a;
print_r($a);
apc_store('table', $a);
}
