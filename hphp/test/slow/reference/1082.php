<?php

function f(&$a) {
}
 class T {
}

 <<__EntryPoint>>
function main_1082() {
$a = new T();
 $a->b = 10;
 f(&$a->b);
 var_dump($a);
}
