<?php

function f(&$a) {
 $a = 'ok';
}
 class T {
}

 <<__EntryPoint>>
function main_1080() {
$a = new T();
 $a->b = 10;
 f(&$a->b);
 var_dump($a);
}
