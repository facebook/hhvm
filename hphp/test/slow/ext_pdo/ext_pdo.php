<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

$tmp_sqllite = tempnam('/tmp', 'vmpdotest');

function createSqliteTestTable($tmp_sqllite) {
  unlink($tmp_sqllite);
  $db = new SQLite3($tmp_sqllite);
  $db->exec("CREATE TABLE foo (bar STRING)");
  $db->exec("INSERT INTO foo VALUES ('ABC')");
  $db->exec("INSERT INTO foo VALUES ('DEF')");
  VS($db->lasterrorcode(), 0);
}

function cleanupSqliteTestTable($tmp_sqllite) {
  unlink($tmp_sqllite);
}

///////////////////////////////////////////////////////////////////////////////

class MyStatement extends PDOStatement {
}

///////////////////////////////////////////////////////////////////////////////

VERIFY(count(pdo_drivers()) > 0);

createSqliteTestTable($tmp_sqllite);

$source = "sqlite:$tmp_sqllite";

try {
  $dbh = new PDO($source);
  $dbh->query("CREATE TABLE IF NOT EXISTS foobar (id INT)");
  $dbh->query("INSERT INTO foobar (id) VALUES (1)");
  $stmt = $dbh->query("SELECT id FROM foobar LIMIT 1");
  $ret = $stmt->fetch();
  VS($ret['id'], "1");

} catch (Exception $e) {
  VS($e, null);
}

try {
  $dbh = new PDO($source);
  VERIFY($dbh != null);
  $stmt = $dbh->prepare("select * from foo");
  VERIFY($stmt != false);
  VERIFY($stmt->execute());

  $rs = $stmt->fetch(PDO::FETCH_ASSOC);
  VS($rs, array("bar" => "ABC"));
  $rs = $stmt->fetch(PDO::FETCH_ASSOC);
  VS($rs, array("bar" => "DEF"));

} catch (Exception $e) {
  VS($e, null);
}

try {
  $dbh = new PDO($source);
  $vstmt = $dbh->query("select * from foo");

  foreach ($vstmt as $k => $v) {
    var_dump($k);
    var_dump($v);
  }

  $vstmt = $dbh->query("select * from foo", 0);
  foreach ($vstmt as $k => $v) {
    var_dump($k);
    var_dump($v);
  }

  //Test column fetching
  $vstmt = $dbh->query("select * from foo", PDO::FETCH_COLUMN, 0);
  var_dump($vstmt->fetchAll());

  class MyShadyObject {
  }

  //Test object fetching
  foreach ($dbh->query("select * from foo", PDO::FETCH_CLASS,
                       'MyShadyObject', NULL) as $row) {
    var_dump($row);
  }

  //Test fetching into an object
  $object = (object)array();
  foreach ($dbh->query("select * from foo", PDO::FETCH_INTO, $object) as $row) {

  }

  //Test bad function calls
  foreach ($dbh->query("select * from foo", PDO::FETCH_INTO) as $row) {

  }

  unset($vstmt);

  //Test setAttribute with ATTR_STATEMENT_CLASS. Set it to our own class
  var_dump($dbh->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('MyStatement')));
  $vstmt = $dbh->query("select * from foo", PDO::FETCH_COLUMN, 0);
  var_dump(get_class($vstmt));
  unset($vstmt);

  //Then reset to PDOStatement. Zend allows the class name to be explicitly set
  //to PDOStatement.
  var_dump($dbh->setAttribute(PDO::ATTR_STATEMENT_CLASS,
                              array('PDOStatement')));
  $vstmt = $dbh->query("select * from foo", PDO::FETCH_COLUMN, 0);
  var_dump(get_class($vstmt));

  unset($dbh);

} catch (Exception $e) {
  VS($e, null);
}

cleanupSqliteTestTable($tmp_sqllite);
