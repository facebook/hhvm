<?php

function foo(&$a) {
}

<<__EntryPoint>>
function main_431() {
$a = array(); foo(&$a[array()]);
foo(&$a[new StdClass]);
var_dump($a);
}
