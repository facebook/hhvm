<?php
function f() {}
$x = 12;
@f();
$y = 34;
@f();
$z = 56;
$t = include 'include2_a.php';
var_dump($x);
var_dump($y);
var_dump($z);
var_dump($t);
$t = include_once 'include2_a.php';
var_dump($t);
$t = @include_once 'include_doesnt_exist.php';
var_dump($t);
$t = include_once 'include2_b.php';
var_dump($t);
