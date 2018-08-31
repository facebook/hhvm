<?php

function t($a, $b = 'k', $c = 'q') {
 var_dump(func_get_args());
}

 <<__EntryPoint>>
function main_1182() {
$a = 'T';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
}
