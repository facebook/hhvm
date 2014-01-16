<?php
require_once __DIR__."/../../utils/server.inc";

$s = new MongoShellServer;
$host = $s->getStandaloneConfig(true);
$creds = $s->getCredentials();

$opts = array(
    "db" => "test",
    "username" => $creds["user"]->username,
    "password" => $creds["user"]->password,
);
$m = new MongoClient($host, $opts);
$db = $m->test2;
$db->authenticate('user2', 'user2' );
$collection = $db->collection;
try
{
	$collection->findOne();
}
catch ( Exception $e )
{
	echo $e->getMessage(), "\n";
}
echo "DONE\n";
?>