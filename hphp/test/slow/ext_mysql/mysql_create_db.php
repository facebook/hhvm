<?php
try {
  mysql_create_db('');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
