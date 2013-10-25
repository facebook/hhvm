<?php
require_once __DIR__.'/../utils/server.inc';

$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array(
    'replicaSet' => $rs['rsname'],
));

$show = false;

MongoLog::setLevel(MongoLog::ALL);
MongoLog::setModule(MongoLog::RS);
MongoLog::setCallback( 'printMsgs' );

function printMsgs($a, $b, $msg)
{
	global $show;

	if (preg_match( '/^pick/', $msg)) {
		$show = true;
	}

	if (preg_match( '/added primary regardless/', $msg) || $show) {
		echo $msg, "\n";
	}
}

// Doesn't match anything
$mc->setReadPreference(MongoClient::RP_PRIMARY_PREFERRED, array(array("dc" => "sf")));
$c = $mc->selectCollection(dbname(), 'bug869');
$c->findOne();


?>