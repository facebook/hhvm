<?php
$a[2][3] = 'stdClass';
$a[$i=0][++$i] =& new $a[++$i][++$i];
print_r($a);

$o = new stdClass;
$o->a =& new $a[$i=2][++$i];
$o->a->b =& new $a[$i=2][++$i];
print_r($o);
?>