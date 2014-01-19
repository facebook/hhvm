<?php

$pdo = new PDO('sqlite::memory:');

$pdo->sqliteCreateAggregate('foo', 'a', '');
$pdo->sqliteCreateAggregate('foo', 'strlen', '');

?>