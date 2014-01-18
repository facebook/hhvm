<?php
require_once __DIR__."/../utils/server.inc";

$cfg = MongoShellServer::getShardInfo();



function log_insert($server, $document, $insert_options) {
    global $newdoc;

    echo __METHOD__, "\n";
    var_dump($server);
    var_dump($newdoc == $document);
    var_dump($insert_options);

    echo "\n\n";
}
function log_query($server, $query, $cursor_options) {
    echo __METHOD__, "\n";

    var_dump($server, $query, $cursor_options);
    echo "\n\n";
}
function log_update($server, $criteria, $newobj, $insert_options, $cursor_options) {
    echo __METHOD__, "\n";

    var_dump($server);
    var_dump($criteria, $newobj, $insert_options, $cursor_options);
    echo "\n\n";
}
function log_delete($server, $criteria, $insert_options, $cursor_options) {
    echo __METHOD__, "\n";

    var_dump($server);
    var_dump($criteria, $insert_options, $cursor_options);
    echo "\n\n";
}



$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_insert" => "log_insert",
            "log_query" => "log_query",
            "log_update" => "log_update",
            "log_delete" => "log_delete",
        )
    )
);
//stream_context_set_params($ctx, array("notification" => "stream_notification_callback", "notifications" => "stream_notification_callback"));

$mc = new MongoClient($cfg[0], array("readPreference" => MongoClient::RP_SECONDARY, "w" => 2), array("context" => $ctx));

$newdoc = array("example" => "document", "with" => "some", "fields" => "in it");
$mc->phpunit->jobs->insert($newdoc);
$mc->phpunit->jobs->find(array("_id" => $newdoc["_id"]))->count();
$obj = $mc->phpunit->jobs->findOne(array("_id" => $newdoc["_id"]), array("with"));
$obj["x"] = time();

$mc->phpunit->jobs->update(array("_id" => $obj["_id"]), $obj, array("w" => 1));
$mc->phpunit->jobs->remove(array("_id" => $obj["_id"]));

?>