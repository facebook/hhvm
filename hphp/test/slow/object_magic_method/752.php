<?php

class A {
 function __call($a, $b) {
 var_dump($a, $b[0], $b[1]);
}
}

 <<__EntryPoint>>
function main_752() {
$obj = new A();
 $a = 1;
 $obj->test($a, 'ss');
}
