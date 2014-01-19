<?php
require_once __DIR__."/../utils/server.inc";
$dsn = MongoShellServer::getStandaloneInfo();

$m = new MongoClient($dsn);
$d = $m->selectDB(dbname());
$c = $d->bug802;

$c->ensureIndex(array('foo' => 1), true);
?>