<?php
require_once __DIR__."/../utils/server.inc";

function log_query($server, $query, $cursor_options) {
    var_dump($query);
}
MongoLog::setLevel(MongoLog::ALL);
MongoLog::setModule(MongoLog::ALL);
MongoLog::setCallback(function($module, $level, $msg) {
    if (strpos($msg, "command supports") !== false) {
        echo $msg, "\n";
        return;
    }
    if (strpos($msg, "forcing") !== false) {
        echo $msg, "\n";
        return;
    }
});


$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_query" => "log_query",
        )
    )
);

$cfg = MongoShellServer::getShardInfo();
$mc = new MongoClient($cfg[0], array("w" => 2, "readPreference" => MongoClient::RP_NEAREST), array("context" => $ctx));

$db = $mc->selectDB(dbname());
$collection = $db->collection;

echo "===This should send 'nearest'===\n";
$db->command(array("distinct" => "people", "key" => "age"));
var_dump($db->getReadPreference());

echo "===These should send 'secondary'===\n";
$db->setReadPreference(MongoClient::RP_SECONDARY);
$db->command(array("distinct" => "people", "key" => "age"));
$db->collection->aggregate(array());
var_dump($db->getReadPreference(), $db->collection->getReadPreference());

echo "===These should send 'primaryPreferred'===\n";
$db->setReadPreference(MongoClient::RP_PRIMARY_PREFERRED);
$db->collection->aggregate(array());
$db->collection->count();
$db->collection->find()->count();
$db->collection->find()->explain();
var_dump($db->getReadPreference(), $db->collection->getReadPreference());


echo "===This should send 'nearest'===\n";
$collection->count(array());
$collection->aggregate(array());
$collection->count(array());
var_dump($collection->getReadPreference());

echo "===These should send 'secondary'===\n";
$collection->setReadPreference(MongoClient::RP_SECONDARY);
$collection->aggregate(array());
$collection->count();
$collection->find()->count();
$collection->find()->explain();
var_dump($collection->getReadPreference());

echo "===These should send 'primaryPreferred'===\n";
$collection->setReadPreference(MongoClient::RP_PRIMARY_PREFERRED);
$collection->aggregate(array());
$collection->find()->count();
$collection->find()->explain();
var_dump($collection->getReadPreference());

?>