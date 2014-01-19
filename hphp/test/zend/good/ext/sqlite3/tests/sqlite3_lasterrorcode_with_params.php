<?php

$db = new SQLite3(':memory:');
var_dump($db->lastErrorCode('invalid argument'));
echo "Done\n";
?>