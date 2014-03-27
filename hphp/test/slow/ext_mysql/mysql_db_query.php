<?php
try {
  mysql_db_query('', '');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
