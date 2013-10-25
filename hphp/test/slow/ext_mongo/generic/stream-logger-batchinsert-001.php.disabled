<?php
require_once __DIR__."/../utils/server.inc";

$dsn = MongoShellServer::getStandaloneInfo();


function log_batchinsert($server, $docs, $info, $options) {
    echo __METHOD__, "\n";

    var_dump($server, count($docs), $info, $options);
}


$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_batchinsert" => "log_batchinsert",
        )
    )
);

$mc = new MongoClient($dsn, array(), array("context" => $ctx));

$docs = array();
foreach(range(0, 200) as $i) {
    $docs[] = array("example" => "document", "with" => "some", "fields" => "in it", "rand" => $i);
}
$opts = array("continueOnError" => 2);
$mc->phpunit->jobs->batchinsert($docs, $opts);
$cursor = $mc->phpunit->jobs->drop();
var_dump($opts);

?>