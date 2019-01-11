<?php

function foo(&$a) {
}

<<__EntryPoint>>
function main_431() {
foo(&$a[array()]);
foo(&$a[new StdClass]);
var_dump($a);
}
