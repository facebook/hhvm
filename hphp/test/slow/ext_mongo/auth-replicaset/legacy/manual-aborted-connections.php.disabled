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
$c = $m->selectCollection("test", "test-ping");

$c->drop();
$c->insert( array( 'test' => 'helium' ) );

for ($i = 0; $i < 20; $i++) {
	$c->insert( array( 'test' => "He$i", 'nr' => $i * M_PI ) );
	try {
		$r = $c->findOne( array( 'test' => "He$i" ) );
	} catch (Exception $e) {
		exit($e->getMessage());
	}
	echo $r['nr'], "\n";
}

?>