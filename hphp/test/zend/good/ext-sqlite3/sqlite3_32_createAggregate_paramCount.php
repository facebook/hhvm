<?php

$db = new SQLite3(':memory:');

$db->createAggregate ();

$db->close();

echo "Done"
?>