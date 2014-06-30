<?php
try {
  mysql_drop_db('');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
