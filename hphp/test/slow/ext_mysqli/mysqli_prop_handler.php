<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

class ExtMySQLi extends mysqli {
  public function __get($name) {
    echo "ExtMySQLi::__get($name);\n";
  }
}

$mysqli = new ExtMySQLi($host, $user, $passwd, $db, $port);

$mysqli->query("DROP TABLE IF EXISTS test4");

if (!$mysqli->query("CREATE TABLE test4 (
  id INTEGER NOT NULL AUTO_INCREMENT,
  test LONGTEXT,
  PRIMARY KEY (id))"
)) {
  die("Error creating table: " . $mysqli->error);
}

$mysqli->query("INSERT INTO test4 (test) VALUES ('test1')");

// Impl-level.

var_dump($mysqli->client_info);
var_dump($mysqli->client_version);
var_dump($mysqli->connect_errno);
var_dump($mysqli->connect_error);
var_dump($mysqli->affected_rows);
var_dump($mysqli->error);
var_dump($mysqli->errno);
var_dump($mysqli->field_count);
var_dump($mysqli->host_info);
var_dump($mysqli->info);
var_dump($mysqli->insert_id);
var_dump($mysqli->protocol_version);
var_dump($mysqli->server_info);
var_dump($mysqli->server_version);
var_dump($mysqli->sqlstate);
var_dump($mysqli->thread_id);
var_dump($mysqli->warning_count);


@$mysqli->query("INSERT INTO test4 (test1) VALUES ('test2')");
var_dump($mysqli->error_list);

// User-level
var_dump($mysqli->nonExisting);

$mysqli->query("DROP TABLE IF EXISTS test4");
