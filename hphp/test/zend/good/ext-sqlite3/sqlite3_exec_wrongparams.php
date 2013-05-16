<?php

$db = new SQLite3(':memory:');
$db->exec(array ('a','b','c'), 20090509);

?>