<?php

$a = array(1);
$a[] =& $a[0];
$a[0] = 2;
print_r($a);

apc_store('table', $a);
$b = apc_fetch('table', $b);
print_r($b);
$b[0] = 3;
print_r($b);

$a = array('xyz' => 'tuv');
$a[] =& $a[0];
$a[0] = 2;
print_r($a);

apc_store('table', $a);
$b = apc_fetch('table', $b);
print_r($b);
$b[0] = 3;
print_r($b);

