<?php
function errorCallBack($c, $m)
{
	echo $m, "\n";
}
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::ALL);
set_error_handler('errorCallBack');
$dsns = array(
	"mongodb://admin:admin@whisky",
	"mongodb://foo:bar@localhost/?replicaSet=seta",
	"mongodb://foo:bar@primary,secondary/?replicaSet=seta",
	"mongodb://foo:bar@primary:14000/database?replicaSet=seta",
	"mongodb://foo:bar@primary:14000/database/?replicaSet=seta",
);

foreach ($dsns as $dsn) {
	echo $dsn, "\n";
	$m = new Mongo($dsn, array('connect' => false));
	echo "\n";
}
?>