<?php
$host   = getenv("MYSQL_TEST_HOST");
$port   = getenv("MYSQL_TEST_PORT");
$user   = getenv("MYSQL_TEST_USER");
$passwd = getenv("MYSQL_TEST_PASSWD");
$db     = getenv("MYSQL_TEST_DB");

$pdo = new PDO("mysql:dbname=$db;host=$host", $user, $passwd);

try {
  $pdo->query("CREATE TABLE test_nextrowset (
    id INT(1),
    PRIMARY KEY(id)
  )");

  $stm = $pdo->query("INSERT INTO test_nextrowset (id) VALUES (1);
               INSERT INTO test_nextrowset (id) VALUES (2), (3);
               INSERT INTO test_nextrowset (id) VALUES (4), (5), (6)");
  for ($i = 0; $i < 3; $i++) {
      var_dump($stm->rowCount());
      var_dump($stm->nextRowset());
  }

} catch (Exception $ex) {
  var_dump($ex);
} finally {
  $pdo->query("DROP TABLE test_nextrowset");
}
