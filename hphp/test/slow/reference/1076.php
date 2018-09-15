<?php

function f(&$a) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1076() {
$a = array();
 $c = &$a['b'];
 $c = 'ok';
 var_dump($a);
}
