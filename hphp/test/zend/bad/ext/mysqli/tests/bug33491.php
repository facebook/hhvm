<?php

class DB extends mysqli
{
  public function query_single($query) {
    $result = parent::query($query);
    $result->fetch_row(); // <- Here be crash
  }
}

require_once("connect.inc");

// Segfault when using the DB class which extends mysqli
$DB = new DB($host, $user, $passwd, $db, $port, $socket);
$DB->query_single('SELECT DATE()');

?>