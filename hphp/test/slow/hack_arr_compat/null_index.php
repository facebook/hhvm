<?php
$t = null[3];
$t = null;
var_dump($t[5]);
$t = false[2];
$arr = ['key' => 'val'];
$t = $arr['invalid key']['invalid key 2'];
$t = $arr['key']['invalid key 2'];

$t = null;
$t[0] = "foo";
$t = null;
$t[1] = "bar";
$t = null;
$t["foo"] = "bar";

$b = false;
$b[0] = "foo";
$b = false;
$b[1] = "foo";
$b = false;
$b["foo"] = "bar";
