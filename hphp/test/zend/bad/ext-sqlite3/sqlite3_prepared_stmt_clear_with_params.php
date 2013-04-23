<?php
$db = new SQLite3(':memory:');
define('TIMENOW', time());
echo "Creating Table\n";
$db->exec('CREATE TABLE test (time INTEGER, id STRING)');
echo "INSERT into table\n";
var_dump($db->exec("INSERT INTO test (time, id) VALUES (" . TIMENOW . ", 'b')"));

echo "SELECTING results\n";
$stmt = $db->prepare("SELECT * FROM test WHERE id = ? ORDER BY id ASC");
var_dump($stmt->clear('invalid argument'));
echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
?>