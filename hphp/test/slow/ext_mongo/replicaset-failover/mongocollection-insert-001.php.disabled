<?php
require_once __DIR__."/../utils/server.inc";

$server = new MongoShellServer;
$rs = $server->getReplicasetConfig();
$mc = new MongoClient($rs["dsn"], array("replicaSet" => $rs["rsname"]));
$coll = $mc->selectCollection("failover", "test1");

for($i = 0; $i < 10; $i++) {
    /* Just to make sure master is working fine */
    try {
        $coll->insert(array("doc" => $i));
    } catch(Exception $e) {
        echo "This was not supposed to happen!!!\n";
        var_dump(get_class($e), $e->getMessage(), $e->getCode());
    }
}
$i++;
echo "Killing master\n";
$server->killMaster();
echo "Master killed\n";
try {
    $coll->insert(array("doc" => $i));
    echo "That query should have failed\n";
} catch(MongoCursorException $e) {
    var_dump($e->getMessage(), $e->getCode());
}
for(; $i < 20; $i++) {
    try {
        $coll->insert(array("doc" => $i));
        echo "That query probably should have failed\n";
    } catch(MongoCursorException $e) {
        var_dump($e->getMessage(), $e->getCode());
    }
}
for($i=0;$i<60; $i++) {
    try {
        $coll->insert(array("doc" => $i));
        echo "Found master again\n";
        break;
    } catch(MongoCursorException $e) {
    }
}
?><?php require_once "__DIR__."/../utils/fix-master.inc"; ?>