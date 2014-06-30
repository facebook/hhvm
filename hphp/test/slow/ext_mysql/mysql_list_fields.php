<?php
try {
  mysql_list_fields('', '');
} catch(Exception $e) {
  echo $e->getMessage(), "\n";
}
