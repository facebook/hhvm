<?php
require_once __DIR__.'/../utils/server.inc';

$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array(
    'replicaSet' => $rs['rsname'],
    'readPreference' => MongoClient::RP_SECONDARY_PREFERRED,
    'readPreferenceTags' => array('foo:bar', ''),
));

echo "MongoClient constructed with empty tag set fallback\n";

$c = $mc->selectCollection(dbname(), 'fixtures');
var_dump($c->findOne());

$mc = new MongoClient($rs['dsn'], array(
    'replicaSet' => $rs['rsname'],
    'readPreference' => MongoClient::RP_SECONDARY_PREFERRED,
    'readPreferenceTags' => array('foo:bar'),
));

echo "MongoClient constructed without empty tag set fallback\n";

$c = $mc->selectCollection(dbname(), 'fixtures');

try {
    echo "Finding one (should fail, we don't have that tag)\n";
    var_dump($c->findOne());
} catch (MongoConnectionException $e) {
    printf("error message: %s\n", $e->getMessage());
}

echo "Secondary read, killing that tag\n";
$c->setReadPreference(MongoClient::RP_SECONDARY_PREFERRED, array());
var_dump($c->findOne());

?>