<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();
$m = new MongoClient($host);

$faulty = 'fo' . chr(0) . 'o';

try {
	$c = $m->selectDb(dbname())->$faulty;
} catch( MongoException $e ) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}

try {
	$c = $m->selectDb(dbname())->selectCollection( $faulty );
} catch( MongoException $e ) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}

try {
	$c = $m->selectCollection( dbname(), $faulty );
} catch( MongoException $e ) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}
?>