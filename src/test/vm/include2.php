<?php
function f() {}
$x = 12;
@f();
$y = 34;
@f();
$z = 56;
include 'include2_a.php';
var_dump($x);
var_dump($y);
var_dump($z);
