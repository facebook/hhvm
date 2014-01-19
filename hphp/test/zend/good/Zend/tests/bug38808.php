<?php
$current = "current";
$next = "next";

$b = array(1=>'one', 2=>'two');
$a =& $b;

echo $current($a)."\n";
$next($a);
echo $current($a)."\n";
?>