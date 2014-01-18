<?php
require_once __DIR__."/../utils/server.inc";

function print_flags($server, $docs, $info, $options) {
    var_dump($info['flags']);
}

$dsn = MongoShellServer::getStandaloneInfo();
$ctx = stream_context_create(array('mongodb' => array('log_batchinsert' => 'print_flags')));

$m = new MongoClient($dsn, array(), array('context' => $ctx));

$c = $m->selectCollection(dbname(), 'batchinsert');
$c->drop();

$documents = array(array('_id' => 1), array('_id' => 2));
$c->batchInsert($documents, array('continueOnError' => 1));

$documents = array(array('_id' => 3), array('_id' => 4));
$c->batchInsert($documents, array('continueOnError' => 0));

var_dump($c->count());
?>