<?php

$matches = 1;
for ($i=1 ; $i < 100000 ; $i++) {
  $db_name = 'dbs.'.rand();

  if (preg_match("/^dbs\.(\d+)$/", $db_name, $match)) {
    $db_num = $match[1];
    $printable_db_name = preg_replace('/' .$db_num.'/', '%d', $db_name);
    if (!$printable_db_name) {
      var_dump("preg_replace returned false");
      break;
    }
    ++$matches;
  }
}
