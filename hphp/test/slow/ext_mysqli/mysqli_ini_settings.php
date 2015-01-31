<?php
$host = getenv("MYSQL_TEST_HOST")   ? getenv("MYSQL_TEST_HOST") : "localhost";
$port = null; // mysqli will use the default 3306
ini_set("mysqli.default_user", "foobar");
$user = ini_get("mysqli.default_user");
$passwd = getenv("MYSQL_TEST_PASSWD") ? getenv("MYSQL_TEST_PASSWD") : "";
$db = getenv("MYSQL_TEST_DB")     ? getenv("MYSQL_TEST_DB") : "test";

// Show that we are using custom credentials
$mysqli = new mysqli($host, $user, $passwd, $db, $port);
if ($mysqli->connect_errno) {
    echo "Failed to connect to MySQL: (" . $mysqli->connect_errno . ") " .
         $mysqli->connect_error;
}
$r = $mysqli->query("DROP TABLE IF EXISTS test");
var_dump($r);

var_dump(ini_get("mysqli.allow_local_infile"));
var_dump(ini_get("mysqli.allow_persistent"));
ini_set("mysqli.max_persistent", 0); // shouldn't be able to do this.
var_dump(ini_get("mysqli.max_persistent"));
var_dump(ini_get("mysqli.max_links"));
var_dump(ini_get("mysqli.default_port"));
var_dump(ini_get("mysqli.default_socket"));
var_dump(ini_get("mysqli.default_host"));
var_dump(ini_get("mysqli.default_user"));
var_dump(ini_get("mysqli.default_pw"));
var_dump(ini_get("mysqli.reconnect"));
var_dump(ini_get("mysqli.cache_size"));
