<?php


<<__EntryPoint>>
function main_545() {
$a1 = array();
$a2 = array(null);
$a3 = array(true);
$a4 = array(false);
$a5 = array(0);
$a6 = array(1);
$a7 = array(1.0);
$a8 = array('1.0');
$a9 = array(1.23456789e+34);
$a13 = array(1.7976931348623157e+308);
$a10 = array(1E666);
$a11 = array(1E666/1E666);
$a12 = array("a bc");
$a13 = array("\xc1 bc");
$a14 = array(null, true, false, 0, 1, 1.0,             '1.0', 1.23456789e+34,             1.7976931348623157e+308, 1E666,             1E666/1E666, "a bc",             "\xc1 bc");
$a15 = array(null => true, false => 0, 1 => 1.0,             '1.0' => 1.23456789e+34,             1.7976931348623157e+308 => 1E666,             1E666/1E666 => "a bc",             "\xc1 bc");
$a16 = array(null => true, false => 0, 1 => 1.0,             '1.0' => 1.23456789e+34,             1.7976931348623157e+308 => 1E666,             1E666/1E666 => "a bc",             "\xc1 bc",             array(null => true, array(),                   false => 0, 1 => 1.0,                   '1.0' => 1.23456789e+34,                   1.7976931348623157e+308 => 1E666,                   1E666/1E666 => "a bc",                   "\xc1 bc"));
var_dump($a1);
var_dump($a2);
var_dump($a3);
var_dump($a4);
var_dump($a5);
var_dump($a6);
var_dump($a7);
var_dump($a8);
var_dump($a9);
var_dump($a10);
var_dump($a11);
var_dump($a12);
var_dump($a13);
var_dump($a14);
var_dump($a15);
var_dump($a16);
}
