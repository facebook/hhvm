<?php

$a = array();
$a[0] = 1;
$a[01] = 2;
$a[007] = 3;
$a[08] = 4;
$a[0xa] = 5;
var_dump($a);
var_dump("$a[0]");
var_dump("$a[1]");
var_dump("$a[7]");
var_dump("$a[10]");
var_dump("$a[01]");
var_dump("$a[007]");
var_dump("$a[08]");
var_dump("$a[0xa]");
