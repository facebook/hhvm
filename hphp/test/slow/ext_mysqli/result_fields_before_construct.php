<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

class MyObject {
  public function __construct() {
    var_dump($this->test); // shoud be available
  }
}

$mysqli = new mysqli($host, $user, $passwd, $db, $port);

$mysqli->query("DROP TABLE IF EXISTS test2");

if (!$mysqli->query("CREATE TABLE test2 (test LONGTEXT)")) {
  die("Error creating table: " . $mysqli->error);
}

if (!$mysqli->query("INSERT INTO test2 (test) VALUES ('test')")) {
  die("Error inserting into the table: " . $mysqli->error);
}

$result = $mysqli->query("SELECT test FROM test2 LIMIT 1");
$obj = $result->fetch_object('MyObject');

$mysqli->query("DROP TABLE IF EXISTS test2");
