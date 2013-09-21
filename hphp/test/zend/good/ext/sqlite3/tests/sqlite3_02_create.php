<?php

require_once(dirname(__FILE__) . '/new_db.inc');

echo "Creating Table\n";
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo "Creating Same Table Again\n";
var_dump($db->exec('CREATE TABLE test (time INTEGER, id STRING)'));

echo "Dropping database\n";
var_dump($db->exec('DROP TABLE test'));

echo "Closing database\n";
var_dump($db->close());
echo "Done\n";
?>