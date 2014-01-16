<?php
require_once __DIR__.'/../utils/server.inc';

$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array('replicaSet' => $rs['rsname']));

$c = $mc->selectCollection(dbname(), 'mongocollection-find_error-002');
$c->insert(array('x' => 1), array('w' => 'majority'));

// Use non-matching tags so query has no candidates
try {
    $c->setReadPreference(MongoClient::RP_SECONDARY, array(array('dc' => 'nowhere')));
    $document = $c->findOne(array('x' => 1), array('_id' => 0));
    var_dump($document);
} catch (MongoConnectionException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

// Use matching tags so query can succeed
try {
    $c->setReadPreference(MongoClient::RP_SECONDARY, array(array('dc' => 'ny')));
    $document = $c->findOne(array('x' => 1), array('_id' => 0));
    var_dump($document);
} catch (MongoConnectionException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

?>