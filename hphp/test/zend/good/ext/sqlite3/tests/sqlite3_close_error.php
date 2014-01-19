<?php

$db = new SQLite3(':memory:');
echo 'Testing SQLite3 close with one parameter' . PHP_EOL;
$db->close('parameter');

echo "Done";
?>