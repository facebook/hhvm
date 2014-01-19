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

    'connect'    => true,
    'timeout'    => 5000,
);
$m = new MongoClient($cfg["dsn"], $opts+array("readPreference" => MongoClient::RP_SECONDARY_PREFERRED));

$database = $m->selectDB('admin');
$command = "db.version()";
$response = $database->execute($command);
var_dump($response);
?>