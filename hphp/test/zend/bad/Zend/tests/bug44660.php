<?php
$s = "hello";
$a = true;

echo "--> read access: ";
echo $a->p;

echo "\n--> direct assignment: ";
$a->p = $s;

echo "\n--> increment: ";
$a->p++;

echo "\n--> reference assignment:";
$a->p =& $s;

echo "\n--> reference assignment:";
$s =& $a->p;

echo "\n--> indexed assignment:";
$a->p[0] = $s;

echo "\n--> Confirm assignments have had no impact:\n";
var_dump($a);
?>