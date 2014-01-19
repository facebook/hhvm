<?php
require_once __DIR__."/../../utils/server.inc";
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

MongoLog::setModule(MongoLog::PARSE);
MongoLog::setLevel(MongoLog::ALL);
$dsn = MongoShellServer::getStandaloneInfo();
$m = new Mongo("mongodb://$dsn");
?>