<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$mysqli = new mysqli($host, $user, $passwd, $db, $port);

if (!$mysqli->query("CREATE TABLE IF NOT EXISTS test1 (test LONGTEXT)") ||
    !$mysqli->query("INSERT INTO test1 (test) VALUES (`test`)")) {
  die("Error creating or inserting table: " . $mysqli->error);
}

$stmt = $mysqli->prepare("SELECT * FROM test1");
$stmt->execute();
echo "Before store_result(): ";
var_dump($stmt->attr_get(MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH));
$stmt->store_result();
echo "\nAfter store_result(): ";
var_dump($stmt->attr_get(MYSQLI_STMT_ATTR_UPDATE_MAX_LENGTH));

$mysqli->query("DROP TABLE IF EXISTS test1");
