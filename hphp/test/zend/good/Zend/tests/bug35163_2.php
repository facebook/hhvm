<?php
$a = array(1);
$b = 'a';
${$b}[] =& $$b;
${$b}[] =& $$b;
${$b}[0] = 2;
var_dump($a);
$a[0] = null;
$a = null;
?>