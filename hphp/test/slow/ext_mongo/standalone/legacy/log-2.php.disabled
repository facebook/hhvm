<?php
require_once __DIR__."/../../utils/server.inc";
function error_handler($code, $message)
{
	echo $message, "\n";
}

set_error_handler('error_handler');

MongoLog::setModule(MongoLog::CON);
MongoLog::setLevel(MongoLog::ALL);


$host = MongoShellServer::getStandaloneInfo();
$m = new Mongo($host);

MongoLog::setModule(0);
MongoLog::setLevel(0);
?>