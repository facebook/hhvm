<?php
require_once __DIR__."/../utils/server.inc";

$server = new MongoShellServer;
$rs = $server->getReplicasetConfig();

function log_query($server, $query, $cursor_options) {
    var_dump($server, $query, $cursor_options);
}
$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_query" => "log_query",
        )
    )
);
$mc = new MongoClient($rs["dsn"], array("replicaSet" => $rs["rsname"]), array("context" => $ctx));

$coll = $mc->selectCollection("ctorfailover", "test1");
$data = array("x" => "The world is not enough");
$coll->insert($data);
$id = $data["_id"];

echo "About to kill master\n";
$server->killMaster();
echo "Master killed\n";


$t = time();
echo "Doing primary read, should fail since we don't have primary\n";
try {
    $coll->findOne(array("_id" => $id));
    echo "Did a primary read without a primary?!\n";
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage(), $e->getCode());
}

echo "Doing secondary read\n";
$data = $coll->setReadPreference(MongoClient::RP_SECONDARY); 
$coll->findOne(array("_id" => $id));

// Since the cleanup is part of this test it can take a while.. this section 
// should definitily not take more then 3secs.
// The only reason we have this here though is to verify we aren't wasting 
// time in attempting to reconnect to master and blocking
var_dump(time()-$t > 3);

?><?php require_once "__DIR__."/../utils/fix-master.inc"; ?>