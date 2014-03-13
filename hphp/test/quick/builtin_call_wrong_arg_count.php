<?php
//////////////////////////////////////////////////////////////////////

function createSqliteTestTable($tmp_sqllite) {
  unlink($tmp_sqllite);
  $db = new SQLite3($tmp_sqllite);
  $db->exec("CREATE TABLE foo (bar STRING)");
  $db->exec("INSERT INTO foo VALUES ('ABC')");
  $db->exec("INSERT INTO foo VALUES ('DEF')");
}

function cleanupSqliteTestTable($tmp_sqllite) {
  unlink($tmp_sqllite);
}

///////////////////////////////////////////////////////////////////////////////

function main() {
  $tmp_sqllite = tempnam('/tmp', 'vmpdotest');
  createSqliteTestTable($tmp_sqllite);

  $source = "sqlite:$tmp_sqllite";

  $dbh = new PDO($source);
  $dbh->query("CREATE TABLE IF NOT EXISTS foobar (id INT)");
  $dbh->query("INSERT INTO foobar (id) VALUES (1)");

  $vstmt = $dbh->prepare("select * from foo");
  var_dump(get_class($vstmt));
  $vstmt->execute(array(), array('a', 'b', 'c'));
  unset($vstmt);

  cleanupSqliteTestTable($tmp_sqllite);
}

main();
