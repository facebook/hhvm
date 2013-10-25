<?php
require_once __DIR__."/../../utils/server.inc";

$s = new MongoShellServer;
$cfg = $s->getReplicaSetConfig(true);
$creds = $s->getCredentials();

$opts = array(
    "db" => "admin",
    "username" => $creds["admin"]->username,
    "password" => $creds["admin"]->password,
    "replicaSet" => $cfg["rsname"],
);
$m = new MongoClient($cfg["dsn"], $opts);

$d = $m->selectDb(dbname());
$c = $d->capped;
$c->drop();

$d->createCollection( 'capped', true, 10*1024, 10 );

for ( $i = 0; $i < 20; $i++ )
{
	$c->insert(array('foo' => $i));
}

$cur = $c->find();
$cur->tailable()->awaitData();

$start = microtime( true );
for ( $i = 0; $i < 12; $i++ )
{
	$cur->getNext();
}
echo microtime( true ) - $start > 2 ? "AWAIT DATA WAITED\n" : "NO WAITING\n";
?>