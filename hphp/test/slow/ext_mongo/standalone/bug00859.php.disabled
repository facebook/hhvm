<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();
$m = new MongoClient($host);
$c = $m->selectDb(dbname())->bug859;
$c->drop();

$document = array('data' => 'test');
$writeConcern = array('w' => 1, 'j' => true);
$c->save($document, $writeConcern);
echo "DONE\n";
?>