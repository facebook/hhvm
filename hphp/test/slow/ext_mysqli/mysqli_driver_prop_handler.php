<?php
$host   = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port   = getenv("MYSQL_TEST_PORT")   ? getenv("MYSQL_TEST_PORT") : 3306;
$user   = getenv("MYSQL_TEST_USER")   ? getenv("MYSQL_TEST_USER") : "root";
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db     = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

class ExtMySQLiDriver extends mysqli_driver {
  public function __get($name) {
    echo "ExtMySQLiDriver::__get($name);\n";
  }

  public function __set($name, $value) {
    echo "ExtMySQLiDriver::__set($name, $value);\n";
  }
}

$driver = new ExtMySQLiDriver();

// Impl-level.

var_dump($driver->client_info);
var_dump(mysqli_get_client_info() === $driver->client_info);

var_dump($driver->client_version);
var_dump(mysqli_get_client_version() === $driver->client_version);

var_dump($driver->driver_version);

var_dump($driver->reconnect);
$driver->reconnect = true;
var_dump($driver->reconnect === true);

var_dump($driver->report_mode);

var_dump(in_array(
  $driver->report_mode,
  array(
    MYSQLI_REPORT_ALL,
    MYSQLI_REPORT_STRICT,
    MYSQLI_REPORT_ERROR,
    MYSQLI_REPORT_INDEX,
    MYSQLI_REPORT_OFF
  )
));

$driver->report_mode = MYSQLI_REPORT_STRICT;
var_dump($driver->report_mode === MYSQLI_REPORT_STRICT);

// User-level
var_dump($driver->nonExisting);
$driver->nonExisting = 1;
