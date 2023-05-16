<?hh

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


<<__EntryPoint>>
function main_ext_sqlite3() {
$db = new SQLite3(sys_get_temp_dir().'/'.':memory:test');
$db->exec("DROP TABLE IF EXISTS foo");
$db->exec("CREATE TABLE foo (bar STRING)");

$db->exec("INSERT INTO foo VALUES ('ABC')");
$db->exec("INSERT INTO foo VALUES ('DEF')");
VS($db->lastinsertrowid(), 2);
VS($db->changes(), 1);
VS($db->lasterrorcode(), 0);
VS($db->lasterrormsg(), "not an error");

VS(SQLite3::escapestring("'\""), "''\"");
VS($db->querysingle("SELECT * FROM foo"), "ABC");
VS($db->querysingle("SELECT * FROM foo", true), darray["bar" => "ABC"]);

// testing query() and SQLite3Result
{
  $res = $db->query("SELECT * FROM foo");

  VS($res->fetcharray(), darray[0 => "ABC", "bar" => "ABC"]);
  VS($res->numcolumns(), 1);
  VS($res->columnname(0), "bar");
  VS($res->columntype(0), SQLITE3_TEXT);

  VS($res->fetcharray(SQLITE3_NUM), darray[0 => "DEF"]);
  $res->finalize();
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
    VS($res->fetcharray(SQLITE3_NUM), darray[0 => "DEF"]);
    $res->finalize();
  }

  VERIFY($stmt->clear());
  VERIFY($stmt->reset());
}

// testing UDF
{
  VERIFY($db->createfunction("tolower", lower<>, 1));
  $res = $db->query("SELECT tolower(bar) FROM foo");
  VS($res->fetcharray(SQLITE3_NUM), darray[0 => "abc"]);
  $res->finalize();
}
{
  VERIFY($db->createaggregate("sumlen", sumlen_step<>, sumlen_fini<>, 1));
  $res = $db->query("SELECT sumlen(bar) FROM foo");
  VS($res->fetcharray(SQLITE3_NUM), darray[0 => 6]);
  $res->finalize();
}

$stmt->close();

// Since minor version can change frequently, just test the major version
VS(SQLite3::version()['versionString'][0], "3");
VERIFY((int)SQLite3::version()['versionNumber'] > (int)3000000);

$db->close();
unlink(sys_get_temp_dir().'/'.':memory:test');

// Check that a PHP Exception is thrown for nonexistant databases
try {
  new SQLite3('/'.uniqid('random', true).'/db');
} catch (Exception $e) {
  var_dump(true);
}
}
