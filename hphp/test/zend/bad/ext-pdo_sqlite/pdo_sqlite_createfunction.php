<?php

$db = new pdo('sqlite::memory:');

$db->query('CREATE TABLE IF NOT EXISTS foobar (id INT AUTO INCREMENT, name TEXT)');

$db->query('INSERT INTO foobar VALUES (NULL, "PHP")');
$db->query('INSERT INTO foobar VALUES (NULL, "PHP6")');


$db->sqliteCreateFunction('testing', function($v) { return strtolower($v); });


foreach ($db->query('SELECT testing(name) FROM foobar') as $row) {
	var_dump($row);
}

$db->query('DROP TABLE foobar');

?>