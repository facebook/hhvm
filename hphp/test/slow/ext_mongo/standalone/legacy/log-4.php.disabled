<?php
require_once __DIR__."/../../utils/server.inc";
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

echo "Warnings:\n";
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::WARNING);
$dsn = MongoShellServer::getStandaloneInfo();
$m = new Mongo("mongodb://$dsn");

echo "Fine:\n";
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::FINE);
$dsn = MongoShellServer::getStandaloneInfo();
$m = new Mongo("mongodb://$dsn");

echo "Info:\n";
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::INFO);
$dsn = MongoShellServer::getStandaloneInfo();
$m = new Mongo("mongodb://$dsn");
MongoLog::setModule(0);
MongoLog::setLevel(0);
?>