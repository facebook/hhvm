<?php

<<__EntryPoint>>
function main_array_keys_for_self() {
$a = array();
$a['self'] = &$a;
var_dump(array_keys($a, $a));
var_dump(array_keys($a, $a, /*strict =*/ TRUE));
}
