<?php

$db = new SQLite3(':memory:');
var_dump($db->close('invalid argument'));
echo "Done\n";
?>