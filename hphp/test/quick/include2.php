<?php
function f() {}
$x = 12;
@f();
$y = 34;
@f();
$z = 56;
$t = include 'include2.1.inc';
var_dump($x);
var_dump($y);
var_dump($z);
var_dump($t);
$t = include_once 'include2.1.inc';
var_dump($t);
$t = @include_once 'include2_doesnt_exist.inc';
var_dump($t);
$t = include_once 'include2.2.inc';
var_dump($t);
