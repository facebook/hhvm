<?php
$db = new SQLite3(':memory:');
var_dump($db);
var_dump($db->changes());
echo "Done\n";
?>