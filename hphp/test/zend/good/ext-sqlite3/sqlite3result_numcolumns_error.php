<?php
$db = new SQLite3(':memory:');
echo 'Creating Table' . PHP_EOL;
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo 'Inserting data' . PHP_EOL;
var_dump($db->exec('INSERT INTO test (time, id) VALUES(2, 1)'));

echo 'Fetching number of columns' . PHP_EOL;
$result = $db->query('SELECT id FROM test');
var_dump($result->numColumns('time'));

echo 'Done';

?>