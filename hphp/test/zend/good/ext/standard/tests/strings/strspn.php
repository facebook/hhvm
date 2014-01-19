<?php
$a = "22222222aaaa bbb1111 cccc";
$b = "1234";
var_dump($a);
var_dump($b);
var_dump(strspn($a,$b));
var_dump(strspn($a,$b,2));
var_dump(strspn($a,$b,2,3));
?>