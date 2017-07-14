<?php

$db = new PDO('sqlite::memory:');
$st = $db->query('SELECT 1');
$re = $st->fetchObject('%Z');
?>
