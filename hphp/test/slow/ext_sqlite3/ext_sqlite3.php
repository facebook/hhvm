<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

function lower($a) {
 return strtolower($a);
}
function sumlen_step($a,$b,$c) {
 return (int)$a + strlen($c);
}
function sumlen_fini($a) {
 return (int)$a;
}

$db = new SQLite3(':memory:');
//$db->open(":memory:test");
$db->exec("DROP TABLE IF EXISTS foo");
$db->exec("CREATE TABLE foo (bar STRING)");

$db->exec("INSERT INTO foo VALUES ('ABC')");
$db->exec("INSERT INTO foo VALUES ('DEF')");
VS($db->lastinsertrowid(), 2);
VS($db->changes(), 1);
VS($db->lasterrorcode(), 0);
VS($db->lasterrormsg(), "not an error");

VS($db->escapestring("'\""), "''\"");
VS($db->querysingle("SELECT * FROM foo"), "ABC");
VS($db->querysingle("SELECT * FROM foo", true), array("bar" => "ABC"));

// testing query() and SQLite3Result
{
  $res = $db->query("SELECT * FROM foo");

  VS($res->fetcharray(), array(0 => "ABC", "bar" => "ABC"));
  VS($res->numcolumns(), 1);
  VS($res->columnname(0), "bar");
  VS($res->columntype(0), SQLITE3_TEXT);

  VS($res->fetcharray(SQLITE3_NUM), array("DEF"));
}

// testing prepare() and sqlite3stmt
{
  $stmt = $db->prepare("SELECT * FROM foo WHERE bar = :id");
  VS($stmt->paramcount(), 1);

  $id = "DEF";
  VERIFY($stmt->bindvalue(":id", $id, SQLITE3_TEXT));
  $id = "ABC";
  {
    $res = $stmt->execute();
    VS($res->fetcharray(SQLITE3_NUM), array("DEF"));
  }

  VERIFY($stmt->clear());
  VERIFY($stmt->reset());
  $id = "DEF";
  VERIFY($stmt->bindparam(":id", $id, SQLITE3_TEXT));
  $id = "ABC";
  {
    $res = $stmt->execute();
    VS($res->fetcharray(SQLITE3_NUM), array("ABC"));
  }
}

// testing UDF
{
  VERIFY($db->createfunction("tolower", "lower", 1));
  $res = $db->query("SELECT tolower(bar) FROM foo");
  VS($res->fetcharray(SQLITE3_NUM), array("abc"));
}
{
  VERIFY($db->createaggregate("sumlen", "sumlen_step", "sumlen_fini", 1));
  $res = $db->query("SELECT sumlen(bar) FROM foo");
  VS($res->fetcharray(SQLITE3_NUM), array(6));
}

// Since minor version can change frequently, just test the major version
VS($db->version()['versionString'][0], "3");
VERIFY((int)$db->version()['versionNumber'] > (int)3000000);

$db->close();
unlink(":memory:test");
