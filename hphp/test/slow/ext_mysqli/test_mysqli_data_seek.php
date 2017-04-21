<?php

// Stemmed from https://github.com/facebook/hhvm/issues/5025

error_reporting(E_ALL);
ini_set('display_errors', 1);
echo $nonexistent;

$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$mysqli = new mysqli($host, $user, $passwd, $db, null, null);
if (!$mysqli->connect_errno) {
    // query with 0 rows
    $res = $mysqli->query("SHOW VARIABLES LIKE '%hhvm_rocks%'");
    var_dump($res->data_seek(0));
}
