<?php
$db = new SQLite3(':memory:');
var_dump($db->loadExtension(array()));
echo "Done\n";
?>