<?php
error_reporting(0);

$a = array(0, "b", "c");

print $a[0];
$a[0] = 1;
print $a[0];
print $a[0]++;
print $a[0];
print "\n";

$a[3]++;
print_r($a);

$b = null;
print $b[0]++;
print "\n";
