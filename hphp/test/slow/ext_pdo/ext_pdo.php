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

  unset($dbh);
  unset($vstmt);

} catch (Exception $e) {
  VS($e, null);
}

cleanupSqliteTestTable($tmp_sqllite);
