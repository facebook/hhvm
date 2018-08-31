<?php

function t($a, $b = 'k') {
 var_dump(func_get_args());
}

 <<__EntryPoint>>
function main_1181() {
$a = 'T';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
}
