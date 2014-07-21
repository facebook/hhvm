<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$pdo = new PDO("mysql:dbname=$db;host=$host", $user, $passwd);

try {
  $pdo->query("CREATE TABLE test_mysql_errcode (
    id INT(1),
    PRIMARY KEY(id)
  )");

  $pdo->query("INSERT INTO test_mysql_errcode (id) VALUES (1)");
  $pdo->query("INSERT INTO test_mysql_errcode (id) VALUES (1)"); // Dupl entry

} catch (PDOException $e) {
  var_dump($e->getCode() == $e->errorInfo[0]);
} finally {
  $pdo->query("DROP TABLE test_mysql_errcode");
}
