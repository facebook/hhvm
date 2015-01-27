<?php

// Stemmed from https://github.com/facebook/hhvm/issues/2082

$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

$c = new mysqli($host, $user, $passwd, $db, $port);
$rs = new mysqli_result($c);
var_dump($c);
var_dump($rs);
$brs = new mysqli_result("Hi"); // Exception thrown here
