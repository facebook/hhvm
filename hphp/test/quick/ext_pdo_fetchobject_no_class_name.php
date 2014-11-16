<?php
$tmp_sqllite = tempnam('/tmp', 'vmpdotest');
$source = "sqlite:$tmp_sqllite";
$db = new PDO($source);
$rows = $db->query('SELECT LENGTH("123456") as col;')->fetchObject();
var_dump($rows);
