<?php

function t($a, $b = 'k') {
 var_dump(func_get_args());
}
 $a = 'T';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
