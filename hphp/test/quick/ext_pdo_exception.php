<?php
try {
    new PDO('aoeu');
} catch (PDOException $e) {
    var_dump($e->errorInfo);
}
$db = new PDO('sqlite::memory:');
$db->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
$db->exec('CREATE TABLE test_table (id INTEGER)');

try {
    $db->exec('CREATE TABLE test_table (id INTEGER)');
} catch (PDOException $ex) {
    var_dump($ex->errorInfo);
}
