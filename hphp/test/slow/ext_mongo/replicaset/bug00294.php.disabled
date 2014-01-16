<?php
require_once __DIR__.'/../utils/server.inc';

function log_query($server, $cursor, $info) {
    var_dump($server["type"] == 4); // 4=secondary
}

$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_query" => "log_query",
        )
    )
);


$rs = MongoShellServer::getReplicasetInfo();
$mc = new MongoClient($rs['dsn'], array('replicaSet' => $rs['rsname']), array("context" => $ctx));

$coll = $mc->selectCollection('phpunit', 'php294');
$coll->drop();
$coll->insert(array('x' => 1), array('w' => 'majority'));

$cmd = $mc->selectCollection('phpunit', '$cmd');
$count = $cmd->findOne(array('count' => 'php294'));
var_dump($count['ok'] && 1 == $count['n']);

$explain = $cmd->find(array('count' => 'php294'))->explain();

$cmd->setReadPreference(MongoClient::RP_SECONDARY);
$explain = $cmd->find(array('count' => 'php294'))->explain();

?>