<?php
require_once __DIR__."/../utils/server.inc";
$dsn = MongoShellServer::getStandaloneInfo();

$m = new MongoClient($dsn);
$d = $m->selectDB(dbname());
$c = $d->bug801;
$c->drop();

$c->insert(array('foo' => 1), true);
?>