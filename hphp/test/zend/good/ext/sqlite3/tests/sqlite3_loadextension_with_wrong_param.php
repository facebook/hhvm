<?php
$db = new SQLite3(':memory:');
try { var_dump($db->loadExtension(array())); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
echo "Done\n";
?>
