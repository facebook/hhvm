<?php
require __DIR__."/../../utils/server.inc";
$config = MongoShellServer::getReplicasetInfo();
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

echo "Warnings:\n";
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::WARNING);
$m = new Mongo($config["hosts"][0], array("replicaSet" => $config["rsname"]));

echo "Fine:\n";
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::FINE);
$m = new Mongo($config["hosts"][0], array("replicaSet" => $config["rsname"]));

echo "Info:\n";
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::INFO);
$m = new Mongo("mongodb://" . $config["hosts"][0], array("replicaSet" => $config["rsname"]));
MongoLog::setModule(0);
MongoLog::setLevel(0);
?>