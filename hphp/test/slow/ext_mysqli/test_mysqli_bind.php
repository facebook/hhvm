<?php

// Stemmed from https://github.com/facebook/hhvm/issues/4505

$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$mysqli = new mysqli($host, $user, $passwd, $db, $port);
if (!$mysqli->query("CREATE TABLE IF NOT EXISTS test (a LONGTEXT, b INT)") ||
    !$mysqli->query("INSERT INTO test (a, b) VALUES ('foo', 1), ('bar', 2)")) {
  die("Error creating or inserting table: " . $mysqli->error);
}

$stmt = $mysqli->prepare("SELECT a, b FROM test WHERE b = ?");
$ref = new ReflectionClass('mysqli_stmt');

$num = 1;
$data = array("i", &$num);
$method = $ref->getMethod('bind_param');
$method->invokeArgs($stmt, $data);

$stmt->execute();

$a = '';
$b = '';
$fields = array(&$a, &$b);
$results = array();
$method = $ref->getMethod('bind_result');
$method->invokeArgs($stmt, $fields);
while($stmt->fetch()) {
  $results[] = unserialize(serialize($fields));
}

var_dump($results);

$mysqli->query("DROP TABLE IF EXISTS test");
$mysqli->close();
