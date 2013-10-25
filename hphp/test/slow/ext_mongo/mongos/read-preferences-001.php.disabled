<?php
require_once __DIR__."/../utils/server.inc";

$cfg = MongoShellServer::getShardInfo();



function log_query($server, $query, $cursor_options) {
    var_dump($query);
    echo "Cursor options (slaveOkay=4): " . $cursor_options["options"], "\n\n";
}


$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_query" => "log_query",
        )
    )
);

$mc = new MongoClient($cfg[0], array("readPreference" => MongoClient::RP_SECONDARY, "readPreferenceTags" => array("dc:ny", "dc:sf"), "w" => 2), array("context" => $ctx));


echo "MongoCollection: RP=secondary, tags=dc:ny,dc:sf\n";
$mc->tests->rp->findOne();


echo "MongoDB: RP=secondary, tags=dc:sf\n";
$db = $mc->tests;
$db->setReadPreference(MongoClient::RP_SECONDARY, array(array("dc" => "sf")));
$db->rp->findOne();

echo "MongoCollection: RP=secondary, tags=server:1\n";
$coll = $db->rp;
$coll->setReadPreference(MongoClient::RP_SECONDARY, array(array("server" => "1")));
$coll->findOne();

echo "MongoDB: RP=secondary, tags=dc:sf (shouldn't have changed after changing the collection)\n";
$db->rp->findOne();

echo "MongoCollection: RP=secondary, tags=dc:ny,dc:sf (shouldn't have changed at all)\n";
$mc->tests->rp->findOne();

echo "MongoDB Changing RP, ignoring tags\n";
$db = $mc->tests;
$db->setReadPreference(MongoClient::RP_SECONDARY);
$db->rp->findOne();

echo "Changing the global MongoClient RP!\n";
$mc->setReadPreference(MongoClient::RP_NEAREST, array(array("dc" => "ny")));

echo "MongoDB: RP=secondary, tags=dc:sf\n";
$db->rp->findOne();

echo "MongoCollection: RP=secondary, tags=server:1\n";
$coll->findOne();

echo "New MongoCollection after changing the MongoClient: RP=nearest, tags=dc:ny\n";
$db = $mc->tests;
$db->rp->findOne();


echo "MongoDB: RP=nearest, tags=dc:ny\n";
$db->rp->findOne();

echo "MongoCollection: RP=nearest, tags=dc:ny\n";
$coll = $db->rp;
$coll->findOne();

?>