<?php

function f(&$a) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1078() {
$a = array();
 f(&$a['b']);
 var_dump($a);
}
