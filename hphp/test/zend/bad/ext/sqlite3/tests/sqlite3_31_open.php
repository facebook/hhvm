<?php

try {
  $db = new SQLite3(__DIR__ . '/db1.db');
  $db->open(__DIR__ . '/db1.db');
} catch (Exception $ex) {
  var_dump($ex->getMessage());
}

?>
<?php @unlink(__DIR__ . '/db1.db'); ?>