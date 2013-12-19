<?php
ini_set('highlight.html', #000000);

$str = '
$x=<<<DD
jhdsjkfhjdsh
DD
."";
$a=<<<DDDD
jhdsjkfhjdsh
DDDD;
';
highlight_string($str);
?>