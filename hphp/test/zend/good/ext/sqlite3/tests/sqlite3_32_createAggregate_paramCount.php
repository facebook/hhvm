<?php

$db = new SQLite3(':memory:');

try { $db->createAggregate (); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

$db->close();

echo "Done";
