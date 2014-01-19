<?php
$a = new stdClass;
$a->b = array(1);
$a->b[] =& $a->b;
$a->b[] =& $a->b;
$a->b[0] = 2;
var_dump($a);
$a->b = null;
$a = null;
?>