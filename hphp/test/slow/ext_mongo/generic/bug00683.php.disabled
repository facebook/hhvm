<?php
require_once __DIR__."/../utils/server.inc";

$dsn = MongoShellServer::getStandaloneInfo();

printLogs(MongoLog::ALL, MongoLog::ALL, "/(Found option.*imeout*)|Replacing/");

echo "timeout only\n";
$m = new MongoClient($dsn, array("connect" => false, "timeout" => 1));

echo "timeout and connectTimeoutMS\n";
$m = new MongoClient($dsn, array("connect" => false, "timeout" => 2, "connectTimeoutMS" => 3));

echo "connectTimeoutMS only\n";
$m = new MongoClient($dsn, array("connect" => false, "connectTimeoutMS" => 4));

echo "connectTimeoutMS and timeout\n";
$m = new MongoClient($dsn, array("connect" => false, "connectTimeoutMS" => 5, "timeout" => 6));

echo "connecttimeoutms lowercased\n";
$m = new MongoClient($dsn, array("connect" => false, "connecttimeoutms" => 7));

?>