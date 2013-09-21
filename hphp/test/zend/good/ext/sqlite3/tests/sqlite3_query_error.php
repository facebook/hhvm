<?php

$db = new SQLite3(':memory:');
echo 'Testing SQLite3 query without parameters' . PHP_EOL;
$db->query();

echo 'Testing SQLite3 query with one array parameter' . PHP_EOL;
$db->query(array());

echo 'Testing SQLite3 qeury with empty string parameter' . PHP_EOL;
var_dump($db->query(''));

echo "Done";
?>