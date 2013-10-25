<?php
require __DIR__."/../../utils/server.inc";
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::ALL);

$config = MongoShellServer::getReplicasetInfo();
$m = new Mongo("mongodb://" . $config["hosts"][0], array("replicaSet" => $config["rsname"]));

MongoLog::setModule(0);
MongoLog::setLevel(0);
?>