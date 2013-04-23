<?php
$db = new SQLite3(':memory:');
var_dump($db->prepare(''));
echo "Done\n";
?>