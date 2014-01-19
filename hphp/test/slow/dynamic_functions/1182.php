<?php

function t($a, $b = 'k', $c = 'q') {
 var_dump(func_get_args());
}
 $a = 'T';
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
