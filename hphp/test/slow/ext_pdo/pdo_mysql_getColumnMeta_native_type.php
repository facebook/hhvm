<?php
$host   = getenv("MYSQL_TEST_HOST");
$port   = getenv("MYSQL_TEST_PORT");
$user   = getenv("MYSQL_TEST_USER");
$passwd = getenv("MYSQL_TEST_PASSWD");
$db     = getenv("MYSQL_TEST_DB");

$pdo = new PDO("mysql:dbname=$db;host=$host", $user, $passwd);

try {
  $pdo->query("CREATE TABLE test_native_type (
    true_false TINYINT(1)
  )");

  $stm = $pdo->query("SELECT true_false FROM test_native_type");
  var_dump($stm->getColumnMeta(0)['native_type']);
} catch (Exception $ex) {
} finally {
  $pdo->query("DROP TABLE test_native_type");
}
