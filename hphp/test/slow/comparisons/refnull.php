<?php


<<__EntryPoint>>
function main_refnull() {
$a = array(null);
$b =&$a[0];
var_dump(in_array(null, $a, true));
}
