<?php
require_once __DIR__."/../utils/server.inc";

$s = new MongoShellServer;
$host = $s->getStandaloneConfig(true);
$creds = $s->getCredentials();

$opts = array(
    "db" => "Xanadu",
    "username" => $creds["user"]->username,
    "password" => $creds["user"]->password,
);
try {
	$m = new MongoClient($host, $opts);
} catch (MongoConnectionException $e) {
	echo $e->getCode(), "\n";
	echo $e->getMessage(), "\n";
}
echo "DONE\n";
?>