<?php

function test($a, $b) {
 var_dump(func_get_args());
}

 <<__EntryPoint>>
function main_1179() {
$a = 'Test';
 $a(1, 2);
 $a(1, 2, 3);
 $a(1, 2, 3, 4);
}
