<?php

$db = new pdo('sqlite::memory:');

$x = $db->query('select 1 as queryStringxx');
$y = $x->fetch(PDO::FETCH_LAZY);
var_dump($y, $y->queryString, $y->queryStringzz, $y->queryStringxx);

print "---\n";

var_dump($y[5], $y->{3});

?>