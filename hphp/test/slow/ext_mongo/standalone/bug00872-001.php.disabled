<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();
$m = new MongoClient($host);

$c = $m->selectDb(dbname())->bug872;
$c->drop();

$document = array(
	'fo' . chr(0) . 'o' => 42,
);

try {
	$c->insert( $document );
} catch( MongoException $e ) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}
?>