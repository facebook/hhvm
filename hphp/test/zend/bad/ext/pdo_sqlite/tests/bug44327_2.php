<?php

$db = new pdo('sqlite::memory:');

$x = $db->query('select 1 as queryString');
var_dump($x, $x->queryString);

$y = $x->fetch();
var_dump($y, @$y->queryString);

print "--------------------------------------------\n";

$x = $db->query('select 1 as queryString');
var_dump($x, $x->queryString);

$y = $x->fetch(PDO::FETCH_LAZY);
var_dump($y, $y->queryString);

?>