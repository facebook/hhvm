<?php
require_once __DIR__."/../utils/server.inc";

$host = MongoShellServer::getStandaloneInfo();

function log_delete($server, $criteria, $flags, $insertopts) {
    echo __METHOD__, "\n";

    var_dump($flags, $insertopts);
}

$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_delete" => "log_delete",
        )
    )
);

$mc = new MongoClient($host, array(), array("context" => $ctx));
$opts = array("justOne" => new stdclass);
$mc->test->col->remove(array(array("doc" => 1)), $opts);
var_dump($opts);
?>