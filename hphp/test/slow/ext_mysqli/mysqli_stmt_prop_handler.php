<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

class ExtMySQLiStmt extends mysqli_stmt {
  public function __get($name) {
    echo "ExtMySQLiStmt::__get($name);\n";
  }
}

class ExtMySQLi extends mysqli {
  public function prepare($query) {
    return new ExtMySQLiStmt($this, $query);
  }
}

$mysqli = new ExtMySQLi($host, $user, $passwd, $db, $port);

$mysqli->query("DROP TABLE IF EXISTS test6");

if (!$mysqli->query("CREATE TABLE test6 (
  id INTEGER NOT NULL AUTO_INCREMENT,
  test LONGTEXT,
  PRIMARY KEY (id))"
)) {
  die("Error creating table: " . $mysqli->error);
}

$mysqli_stmt = $mysqli->prepare("INSERT INTO test6 (test) VALUES ('test1')");
$mysqli_stmt->execute();

function test($mysqli_stmt, $method) {
  echo '$mysqli_stmt->'.$method.': ';
  var_dump($mysqli_stmt->$method);
}

// Impl-level.

test($mysqli_stmt, 'affected_rows');
test($mysqli_stmt, 'errno');
test($mysqli_stmt, 'error');
test($mysqli_stmt, 'field_count');
test($mysqli_stmt, 'insert_id');
test($mysqli_stmt, 'num_rows');
test($mysqli_stmt, 'param_count');
test($mysqli_stmt, 'sqlstate');

$mysqli_stmt = $mysqli->prepare("INSERT INTO test6 (test1) VALUES ('test2')");
test($mysqli_stmt, 'error_list');

// User-level
test($mysqli_stmt, 'nonExisting');

$mysqli->query("DROP TABLE IF EXISTS test6");
