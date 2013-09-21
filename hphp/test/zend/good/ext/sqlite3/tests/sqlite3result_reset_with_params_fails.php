<?php
$db = new SQLite3(':memory:');
$db->exec('CREATE TABLE foo (bar STRING)');
$db->exec("INSERT INTO foo (bar) VALUES ('This is a test')");
$db->exec("INSERT INTO foo (bar) VALUES ('This is another test')");

$result = $db->query('SELECT bar FROM foo');
$result->reset(1);
?>