<?php

function f(&$a) {
}

 <<__EntryPoint>>
function main_1081() {
$a = array();
 f(&$a['b']);
 var_dump($a);
}
