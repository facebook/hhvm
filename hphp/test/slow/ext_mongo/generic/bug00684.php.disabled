<?php
require_once __DIR__."/../utils/server.inc";

$dsn = MongoShellServer::getStandaloneInfo();

printLogs(MongoLog::ALL, MongoLog::ALL, "/timeout/i");

echo "socketTimeoutMS (option)\n";
$m = new MongoClient($dsn, array("socketTimeoutMS" => 1, "connect" => false));

echo "sockettimeoutms lowercased (option)\n";
$m = new MongoClient($dsn, array("socketTimeoutMS" => 2, "connect" => false));

echo "socketTimeoutMS (string)\n";
$m = new MongoClient("localhost/?socketTimeoutMS=42", array("connect" => false));

echo "sockettimeoutms lowercased (string)\n";
$m = new MongoClient("localhost/?sockettimeoutms=52", array("connect" => false));

echo "socketTimeoutMS (string and option)\n";
$m = new MongoClient("localhost/?socketTimeoutMS=62", array("connect" => false, 'socketTimeoutMS' => 72));
?>