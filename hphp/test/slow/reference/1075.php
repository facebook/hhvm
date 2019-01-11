<?php

function f(&$a) {
 $a = 'ok';
}

 <<__EntryPoint>>
function main_1075() {
$a = 10;
 f(&$a);
 var_dump($a);
}
