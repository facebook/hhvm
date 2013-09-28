<?php

function t($a = 'k') {
 var_dump(func_get_args());
}
 $a = 'T';
 $a();
 $a('o');
 $a('o', 'p');
 $a('o', 'p', 'q');
