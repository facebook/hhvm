<?php
require_once __DIR__."/../utils/server.inc";

$dsn = MongoShellServer::getStandaloneInfo();
printLogs(MongoLog::ALL, MongoLog::ALL, "/(Found option.*imeout*)|Replacing/");

echo "wTimeout only\n";
$m = new MongoClient($dsn, array("connect" => false, "wTimeout" => 1));

echo "wTimeout and wTimeoutMS\n";
$m = new MongoClient($dsn, array("connect" => false, "wTimeout" => 2, "wTimeoutMS" => 3));

echo "wTimeoutMS only\n";
$m = new MongoClient($dsn, array("connect" => false, "wTimeoutMS" => 4));

echo "wTimeoutMS and wTimeout\n";
$m = new MongoClient($dsn, array("connect" => false, "wTimeoutMS" => 5, "wTimeout" => 6));

echo "wtimeoutms lowercased\n";
$m = new MongoClient($dsn, array("connect" => false, "wtimeoutms" => 7));

?>