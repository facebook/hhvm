<?php

function t($a = 'k') {
 var_dump(func_get_args());
}

 <<__EntryPoint>>
function main_1180() {
$a = 'T';
 $a();
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
}
