<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();

function log_update($server, $old, $newobj, $flags, $insertopts) {
    echo __METHOD__, "\n";

    var_dump($flags, $insertopts);
}

$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_update" => "log_update",
        )
    )
);

$mc = new MongoClient($host, array(), array("context" => $ctx));
$opts = array("upsert" => new stdclass, "multiple" => new stdclass);
$mc->test->col->update(array(array("doc" => 1)), array('$set' => array("doc" => 2)), $opts);
var_dump($opts);
$opts = array("upsert" => new stdclass);
$mc->test->col->update(array(array("doc" => 1)), array('$set' => array("doc" => 2)), $opts);
$opts = array("multiple" => new stdclass);
$mc->test->col->update(array(array("doc" => 1)), array('$set' => array("doc" => 2)), $opts);
?>