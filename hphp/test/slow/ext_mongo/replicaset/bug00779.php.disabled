<?php
require_once __DIR__.'/../utils/server.inc';

$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array('replicaSet' => $rs['rsname']));

$c = $mc->selectCollection(dbname(), 'bug00779');
$c->drop();
$c->insert(array('x' => 1), array('w' => 'majority'));

function check_slaveOkay($server, $query, $cursor_options)
{
    printf("Bit 2 (SlaveOk) is%s set\n", ($cursor_options['options'] & 1 << 2) ? '' : ' not');
}

$ctx = stream_context_create(array('mongodb' => array('log_query' => 'check_slaveOkay')));

// Connect to secondary in standalone mode
$mc = new MongoClient($rs['hosts'][1], array(), array('context' => $ctx));

$coll = $mc->selectCollection(dbname(), 'bug00779');

echo "Testing primary query with MongoCursor::setReadPreference()\n";
$cursor = $coll->find();
$cursor->setReadPreference(MongoClient::RP_PRIMARY);
try {
    iterator_to_array($cursor);
} catch (MongoCursorException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

echo "\nTesting primary query with MongoCursor::setFlag()\n";
$cursor = $coll->find();
$cursor->setFlag(2, false);
try {
    iterator_to_array($cursor);
} catch (MongoCursorException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

echo "\nTesting primary query\n";
$cursor = $coll->find();
try {
    iterator_to_array($cursor);
} catch (MongoCursorException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

echo "\nTesting primary count\n";
try {
    /* TODO: this will return an error document instead of throwing an
     * exception. Fix the expected output once PHP-781 is resolved.
     */
    $coll->count();
} catch (MongoException $e) {
    var_dump($e->getMessage(), $e->getCode());
}

echo "\n----\n";

echo "\nTesting non-primary query with MongoCursor::setReadPreference()\n";
$cursor = $coll->find();
$cursor->setReadPreference(MongoClient::RP_SECONDARY_PREFERRED);
iterator_to_array($cursor);

echo "\nTesting non-primary query with MongoCursor::setFlag()\n";
$cursor = $coll->find();
$cursor->setFlag(2);
iterator_to_array($cursor);

echo "\nTesting non-primary query with MongoCollection::setReadPreference()\n";
$coll->setReadPreference(MongoClient::RP_SECONDARY_PREFERRED);
$cursor = $coll->find();
iterator_to_array($cursor);

echo "\nTesting non-primary count with MongoCollection::setReadPreference()\n";
$coll->count();

?>