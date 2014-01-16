<?php
require_once __DIR__."/../../utils/server.inc";

function log_query($server, $query, $cursor_options) {
    echo $server["type"] == 2 ?  "Hit the primary\n" : "Hit a secondary\n";
}
$ctx = stream_context_create(
    array(
        "mongodb" => array(
            "log_query" => "log_query",
        )
    )
);

$cfg = MongoShellServer::getReplicasetInfo();
$m = new MongoClient($cfg["dsn"], array("replicaSet" => $cfg["rsname"]), array("context" => $ctx));


$db = $m->selectDB(dbname());
$db->dropCollection("fs.files");
$db->dropCollection("fs.chunks");

$gridfs = $db->getGridFS();

for($i=0; $i<5; $i++) {
    // Since we will be reading from slave in a second, it is nice to know that the file is there
    $safe = array("safe" => 1, "w" => "majority");
    try {
        $ok = $gridfs->storeFile(__FILE__, array( "_id" => "slaveOkayFile-$i"), $safe);
    } catch(Exception $e) {
        var_dump("Failed writing it ($i)");
    }
    var_dump($ok);
}
$bytes = strlen(file_get_contents(__FILE__));

$db = $m->selectDB(dbname());
$gridfs = $db->getGridFS();
$cursor = $gridfs->find()->slaveOkay(true);
$cursor->count();

foreach($cursor as $file) {
}
?>
===DONE===