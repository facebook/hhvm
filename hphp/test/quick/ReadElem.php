<?php

error_reporting(0);

$a = array(10, 20, 30);
print $a[0] . "\n";
print $a[1] . "\n";
print $a[2] . "\n";
print $a[3];# . "\n";
print $a["0"] . "\n";
print $a["x"];# . "\n";

$s = "abc";
print $s[0] . "\n";
print $s[1] . "\n";
print $s[2] . "\n";
print $s[3];# . "\n";
print $s["0"] . "\n";
print $s["x"] . "\n";
