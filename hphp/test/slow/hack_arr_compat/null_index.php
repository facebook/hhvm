<?php


<<__EntryPoint>>
function main_null_index() {
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

echo "=== assigning to an index of true ===\n";
$b = true;
$b[0] = "foo";
$b = true;
$b[1] = "foo";
$b = true;
$b["foo"] = "bar";
}
