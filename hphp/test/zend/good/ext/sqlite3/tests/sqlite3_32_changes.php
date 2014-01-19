<?php
$db = new SQLite3(':memory:');
//$db = new SQLite3('mysqlitedb.db');
$db->exec('CREATE TABLE pageView(id INTEGER PRIMARY KEY, page CHAR(256), access INTEGER(10))');
$db->exec('INSERT INTO pageView (page, access) VALUES (\'test\', \'000000\')');
echo $db->changes("dummy");
?>