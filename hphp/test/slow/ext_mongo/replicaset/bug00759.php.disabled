<?php
require_once __DIR__.'/../utils/server.inc';

$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array(
    'replicaSet' => $rs['rsname'],
    'readPreference' => MongoClient::RP_PRIMARY_PREFERRED,
    'readPreferenceTags' => array('dc:ny'),
));

// Doesn't match anything
$mc->setReadPreference(MongoClient::RP_SECONDARY, array(array("foo" => "bar")));

$c = $mc->selectDb(dbname())->fixtures;
$c->setReadPreference(MongoClient::RP_SECONDARY_PREFERRED, array());
var_dump($c->findOne(array("example" => "document")));

try {
    $mc->selectDB(dbname())->random->drop();
    echo "Drop succeeded as it should\n";
} catch(Exception $e) {
    echo "FAILED\n";
    var_dump(get_class($e), $e->getMessage(), $e->getCode());
}

?>