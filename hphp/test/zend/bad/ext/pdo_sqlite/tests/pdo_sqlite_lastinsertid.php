<?php

$db = new pdo('sqlite::memory:');
$db->query('CREATE TABLE IF NOT EXISTS foo (id INT AUTO INCREMENT, name TEXT)');
$db->query('INSERT INTO foo VALUES (NULL, "PHP")');
$db->query('INSERT INTO foo VALUES (NULL, "PHP6")');
var_dump($db->query('SELECT * FROM foo'));
var_dump($db->errorInfo());
var_dump($db->lastInsertId());

$db->query('DROP TABLE foo');

?>