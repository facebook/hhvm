<?php
require_once __DIR__."/../utils/server.inc";

$server = new MongoShellServer;
$rs = $server->getReplicasetConfig();

function log_query($server, $query, $cursor_options) {
    printf("Server type: %s (%d)\n", $server["type"] == 2 ? "PRIMARY" : ($server["type"] == 4 ? "SECONDARY" : "UNKNOWN"), $server["type"]);
}

$ctx = stream_context_create(array("mongodb" => array("log_query" => "log_query")));

$mc = new MongoClient($rs["dsn"], array("replicaSet" => $rs["rsname"], "readPreference" => MongoClient::RP_SECONDARY_PREFERRED), array("context" => $ctx));
$i = "random";
$mc->selectDb(dbname())->recoverymode->insert(array("doc" => $i, "w" => "majority"));

// Lower the ismaster interval to make sure we pick up on the change
ini_set("mongo.is_master_interval", 1);

echo "Putting all secondaries into recovery mode\n";
$server->setMaintenanceForSecondaries(true);
sleep(3);

echo "We should have detected that the servers are in maintenence mode now\n";
echo "This should hit the primary as all secondaries are in recovery\n";
for($i=0; $i < 10; $i++) {
    try {
        $mc->selectDb(dbname())->recoverymode->findOne(array("doc" => $i));
    } catch(MongoCursorException $e) {
        var_dump($e->getMessage(), $e->getCode());
    }
}

echo "Enabling all secondaries again\n";
$server->setMaintenanceForSecondaries(false);
sleep(3);

echo "This should hit secondaries as they are no longer in recovery\n";
for($i=0; $i < 10; $i++) {
    try {
        $mc->selectDb(dbname())->recoverymode->findOne(array("doc" => $i));
    } catch(MongoCursorException $e) {
        var_dump($e->getMessage(), $e->getCode());
    }
}

// Increase the interval so we don't notice the change until we hit the servers
ini_set("mongo.is_master_interval", 50);
echo "Putting all secondaries into recovery mode\n";
$server->setMaintenanceForSecondaries(true);

echo "These should throw exception as we haven't detected that the server is in recovery mode yet\n";
/* Once per secondary */
for($i=0; $i < 3; $i++) {
    try {
        $mc->selectDb(dbname())->recoverymode->findOne(array("doc" => $i));
    } catch(MongoCursorException $e) {
        var_dump(get_class($e), $e->getMessage(), $e->getCode());
    }
}

echo "This should hit the primary as all secondaries are in recovery\n";
for($i=0; $i < 10; $i++) {
    try {
        $mc->selectDb(dbname())->recoverymode->findOne(array("doc" => $i));
    } catch(MongoCursorException $e) {
        var_dump($e->getMessage(), $e->getCode());
    }
}

echo "Enabling all secondaries again\n";
$server->setMaintenanceForSecondaries(false);
echo "Everything should be in its original state now\n";

?><?php require_once "__DIR__."/../utils/fix-secondaries.inc"; ?>