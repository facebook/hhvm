<?php
$db = new SQLite3(':memory:');
var_dump($db->lastErrorMsg('invalid argument'));
echo "Done\n";
?>