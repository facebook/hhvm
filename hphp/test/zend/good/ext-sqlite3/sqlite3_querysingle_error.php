<?php
$db = new SQLite3(':memory:');
echo 'Testing SQLite3 querySingle without parameters' . PHP_EOL;
$db->querySingle();

echo 'Testing SQLite3 querySingle with one array parameter' . PHP_EOL;
$db->querySingle(array());

echo 'Testing SQLite3 qeurySingle with empty string parameter' . PHP_EOL;
var_dump($db->querySingle(''));

echo "Done";
?>