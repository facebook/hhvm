<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();
$m = new MongoClient($host);
$c = $m->selectDb(dbname())->bug853;
$c->drop();

$document = array(
	'foo' => 42,
	'broken-utf8' => 'F' . chr( 180 ),
);

try {
	$c->insert( $document );
} catch( MongoException $e ) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}

try {
	$c->batchInsert( array( $document ) );
} catch( MongoException $e ) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}

?>