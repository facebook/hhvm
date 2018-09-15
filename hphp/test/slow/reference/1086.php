<?php

function &f() {
 $a = array();
 return $a['b'];
}

 <<__EntryPoint>>
function main_1086() {
$b = &f();
 $b = 20;
 var_dump($b);
}
