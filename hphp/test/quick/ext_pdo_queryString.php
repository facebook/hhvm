<?php
$tmp_sqllite = tempnam('/tmp', 'vmpdotest');
$source = "sqlite:$tmp_sqllite";
$dbh = new PDO($source);
$stmt = $dbh->query("SELECT 1+1");
echo $stmt->queryString;

unset($stmt);
unset($dbh);
unlink($tmp_sqllite);
