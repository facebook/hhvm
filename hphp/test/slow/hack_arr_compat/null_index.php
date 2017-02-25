<?php

echo "=== indexing into null ===\n";
$t = null[3];
$t = null;
var_dump($t[5]);

echo "=== indexing into a boolean ===\n";
$t = false[2];

echo "=== indexing into invalid keys ===\n";
$arr = ['key' => 'val'];
$t = $arr['invalid key']['invalid key 2'];
$t = $arr['key']['invalid key 2'];

echo "=== assigning to an index of null ===\n";
$t = null;
$t[0] = "foo";
$t = null;
$t[1] = "bar";
$t = null;
$t["foo"] = "bar";

echo "=== assigning to an index of false ===\n";
$b = false;
$b[0] = "foo";
$b = false;
$b[1] = "foo";
$b = false;
$b["foo"] = "bar";

echo "=== assigning to an index of true ===\n";
$b = true;
$b[0] = "foo";
$b = true;
$b[1] = "foo";
$b = true;
$b["foo"] = "bar";
