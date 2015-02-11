<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$mysqli = new mysqli($host, $user, $passwd, $db, $port);

$mysqli->query("DROP TABLE IF EXISTS test3");

if (!$mysqli->query("CREATE TABLE test3 (test LONGTEXT)")) {
  die("Error creating table: " . $mysqli->error);
}

if (!$mysqli->query("INSERT INTO test3 (test) VALUES ('test')")) {
  die("Error inserting into the table: " . $mysqli->error);
}

// Existing.

$result = $mysqli->query("SELECT test FROM test3 LIMIT 1");
$result->fetch_object();

var_dump(isset($result->num_rows));
var_dump($result->num_rows);

var_dump(isset($result->current_field));
var_dump($result->current_field);

var_dump(isset($result->field_count));
var_dump($result->field_count);

var_dump(isset($result->lengths));
var_dump($result->lengths);

// Non-existing.

$result = $mysqli->query("SELECT test FROM test3 where test = 'foo' LIMIT 1");
$result->fetch_object();

var_dump(isset($result->num_rows));
var_dump($result->num_rows);

var_dump(isset($result->current_field));
var_dump($result->current_field);

var_dump(isset($result->field_count));
var_dump($result->field_count);

var_dump(isset($result->lengths));
var_dump($result->lengths);

$mysqli->query("DROP TABLE IF EXISTS test3");
