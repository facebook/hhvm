<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) {
    echo "Failed: $y\n";
    echo "Got: $x\n";
    var_dump(debug_backtrace());
  }
}
function VERIFY($x) :mixed{
  VS($x != false, true);
}

function createSqliteTestTable($tmp_sqllite) :mixed{
  unlink($tmp_sqllite);
  $db = new SQLite3($tmp_sqllite);
  $db->exec("CREATE TABLE foo (bar STRING)");
  $db->exec("INSERT INTO foo VALUES ('ABC')");
  $db->exec("INSERT INTO foo VALUES ('DEF')");
  VS($db->lasterrorcode(), 0);
}

function cleanupSqliteTestTable($tmp_sqllite) :mixed{
  unlink($tmp_sqllite);
}

///////////////////////////////////////////////////////////////////////////////

class MyStatement extends PDOStatement {
}


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_ext_pdo() :mixed{
  $tmp_sqllite = tempnam(sys_get_temp_dir(), 'vmpdotest');

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
    VS($rs, dict["bar" => "ABC"]);
    $rs = $stmt->fetch(PDO::FETCH_ASSOC);
    VS($rs, dict["bar" => "DEF"]);

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

    include 'ext_pdo.inc';

    //Test object fetching
    foreach (
      $dbh->query(
        "select * from foo",
        PDO::FETCH_CLASS,
        'MyShadyObject',
        NULL,
      ) as $row
    ) {
      var_dump($row);
    }

    //Test fetching into an object
    $object = new stdClass();
    foreach (
      $dbh->query("select * from foo", PDO::FETCH_INTO, $object) as $row
    ) {

    }

    //Test bad function calls
    try {
      foreach ($dbh->query("select * from foo", PDO::FETCH_INTO) as $row) {
      }
    } catch (InvalidForeachArgumentException $e) {
      var_dump($e->getMessage());
    }

    unset($vstmt);

    //Test setAttribute with ATTR_STATEMENT_CLASS. Set it to our own class
    var_dump(
      $dbh->setAttribute(PDO::ATTR_STATEMENT_CLASS, vec['MyStatement']),
    );
    $vstmt = $dbh->query("select * from foo", PDO::FETCH_COLUMN, 0);
    var_dump(get_class($vstmt));
    unset($vstmt);

    //Then reset to PDOStatement. Zend allows the class name to be explicitly set
    //to PDOStatement.
    var_dump(
      $dbh->setAttribute(PDO::ATTR_STATEMENT_CLASS, vec['PDOStatement']),
    );
    $vstmt = $dbh->query("select * from foo", PDO::FETCH_COLUMN, 0);
    var_dump(get_class($vstmt));

    unset($dbh);

  } catch (Exception $e) {
    VS($e, null);
  }

  cleanupSqliteTestTable($tmp_sqllite);
}
