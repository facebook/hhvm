<?php

$db = new pdo('sqlite::memory:');

$x= $db->prepare('select :a, :b, ?');
$x->bindValue(':a', 1, PDO::PARAM_INT);
$x->bindValue(':b', 'foo');
$x->bindValue(3, 1313);
var_dump($x->debugDumpParams());

?>