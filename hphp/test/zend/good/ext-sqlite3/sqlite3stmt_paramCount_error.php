<?php

require_once(dirname(__FILE__) . '/new_db.inc');

echo "Creating Table\n";
var_dump($db->exec('CREATE TABLE foobar (id INTEGER, name STRING, city STRING)'));

echo "INSERT into table\n";
var_dump($db->exec("INSERT INTO foobar (id, name, city) VALUES (1, 'john', 'LA')"));
var_dump($db->exec("INSERT INTO foobar (id, name, city) VALUES (2, 'doe', 'SF')"));


$query = "SELECT * FROM foobar WHERE id = ? ORDER BY id ASC";

echo "SELECTING results\n";

$stmt = $db->prepare($query);

echo "paramCount with wrong number of arguments\n";
var_dump($stmt->paramCount('foobar'));
$result = $stmt->execute();
echo "Closing database\n";
$stmt = null;
$result = null;
var_dump($db->close());
echo "Done\n";
?>