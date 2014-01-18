<?php
require_once __DIR__."/../../utils/server.inc";

function get_user($m, $username) {
    $db = $m->selectDB("admin");
    $c = $db->selectCollection("system.users");

    return $c->findOne(array("user" => $username));
}

$s = new MongoShellServer;
$cfg = $s->getReplicaSetConfig(true);
$creds = $s->getCredentials();

$opts = array(
    "db" => "admin",
    "username" => $creds["admin"]->username,
    "password" => $creds["admin"]->password,
    "replicaSet" => true,
);
$m = new MongoClient($cfg["dsn"], $opts+array("readPreference" => MongoClient::RP_SECONDARY_PREFERRED));
var_dump($m);
var_dump(get_user($m, $creds["admin"]->username));

try {
    $opts["password"] .= "THIS-PASSWORD-IS-WRONG";
    printLogs(MongoLog::CON, MongoLog::WARNING);
    $m = new MongoClient($cfg["dsn"], $opts+array("readPreference" => MongoClient::RP_SECONDARY_PREFERRED));
    echo "I still have a MongoClient object\n";
    $m->admin->test->findOne();
} catch (MongoConnectionException $e) {
    echo $e->getMessage(), "\n";
}

?>