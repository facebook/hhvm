<?php

function test($a) {
 var_dump(func_get_args());
}

 <<__EntryPoint>>
function main_1178() {
$a = 'Test';
 $a(1);
 $a(1, 2);
}
