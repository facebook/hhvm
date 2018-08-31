<?php

function &f() {
 $a = 10;
 return $a;
}

 <<__EntryPoint>>
function main_1084() {
$b = &f();
 $b = 20;
 var_dump($b);
}
