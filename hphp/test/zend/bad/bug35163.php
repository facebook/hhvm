<?php
$a = array(array(1));
$a[0][] =& $a[0];
$a[0][] =& $a[0];
$a[0][0] = 2;
var_dump($a);
$a[0] = null;
$a = null;
?>