<?php

# Static arrays.
$a = array();
var_dump($a);

$a = array(null);
var_dump($a);

$a = array(true);
var_dump($a);

$a = array(42);
var_dump($a);

$a = array(12.34);
var_dump($a);

$a = array("hello");
var_dump($a);

$a = array(array());
var_dump($a);

$a = array(null, true, 42, 12.34, "hello", array(1, array(2, array(3))));
var_dump($a);
$a = array(null, true, 42, 12.34, "hello", array(1, array(2, array(3))));
var_dump($a);

$a = array(null => "null");
var_dump($a);

$a = array(false => "false");
var_dump($a);

$a = array(true => "true");
var_dump($a);

$a = array(0 => "0");
var_dump($a);

$a = array(42 => "42");
var_dump($a);

$a = array(12.34 => "12.34");
var_dump($a);

$a = array("hello" => "world");
var_dump($a);

$a = array(0 => "0", true => "1", "hello" => "world", 12.34 => array());
var_dump($a);

# Non-static arrays.
$v = null;
$a = array($v);
var_dump($a);

$k = 0;
$a = array($k => "0");
var_dump($a);

$v = "0";
$a = array(0 => $v);
var_dump($a);

$k = "hello";
$a = array($k => "world");
var_dump($a);

$v = "world";
$a = array("hello" => $v);
var_dump($a);

$v = 0;
$a = array(array($v));
var_dump($a);

$v = 0;
$a = array(array($v), array(0));
var_dump($a);

$v = 0;
$a = array(array(0), array($v));
var_dump($a);

# Invalid key, prevents static array optimization.
$a = array(array() => 1);
var_dump($a);

$a = array(INF => 0);
var_dump($a);

$a = array(NAN => 0);
var_dump($a);

