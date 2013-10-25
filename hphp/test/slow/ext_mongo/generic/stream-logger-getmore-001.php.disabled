<?php
require_once __DIR__."/../utils/server.inc";

$dsn = MongoShellServer::getStandaloneInfo();


function log_getmore($server, $cursor_options) {
    echo __METHOD__, "\n";

    var_dump($server, $cursor_options);
}


$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_getmore" => "log_getmore",
        )
    )
);

$mc = new MongoClient($dsn, array(), array("context" => $ctx));

foreach(range(0, 200) as $i) {
    $newdoc = array("example" => "document", "with" => "some", "fields" => "in it", "rand" => $i);
    $mc->phpunit->jobs->insert($newdoc);
}
$cursor = $mc->phpunit->jobs->find();
$i = 0;
foreach($cursor as $doc) {
    if (++$i == 101) {
        echo "Now I should getmore\n";
    } elseif ($i == 102) {
        echo "There should be a getmore query above\n";
    }
}
$cursor = $mc->phpunit->jobs->drop();


?>