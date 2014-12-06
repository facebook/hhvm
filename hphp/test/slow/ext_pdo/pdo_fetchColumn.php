<?php

$tmp_sqlite = tempnam('/tmp', 'vmpdotest');

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}

function createSqliteTestTable($tmp_sqlite) {
  unlink($tmp_sqlite);
  $db = new SQLite3($tmp_sqlite);
  $db->exec("CREATE TABLE foo (bar STRING)");
  $db->exec("INSERT INTO foo VALUES ('ABC')");
  $db->exec("INSERT INTO foo VALUES ('DEF')");
  VS($db->lasterrorcode(), 0);
}

function cleanupSqliteTestTable($tmp_sqlite) {
  unlink($tmp_sqlite);
}

createSqliteTestTable($tmp_sqlite);

$source = "sqlite:$tmp_sqlite";

try {
  $dbh = new PDO($source);
  $result = $dbh->query('SELECT * FROM foo');
  var_dump($result->fetchColumn());
  var_dump($result->fetchColumn(0));
  var_dump($result->fetchColumn(3));
} finally {
  cleanupSqliteTestTable($tmp_sqlite);
}
