<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$pdo = new PDO("mysql:dbname=$db;host=$host", $user, $passwd);

try {
  $statement = $pdo->prepare("SELECT @var := (2-1)");

  for ($i=0; $i<3; $i++) {
    var_dump($statement->execute());
    var_dump($statement->fetchColumn(0));
  }
} catch (PDOException $e) {
  echo "Caught exception $e\n";
}
