<?php
require_once __DIR__."/../utils/server.inc";

$rs = MongoShellServer::getReplicasetInfo();

try {
    /* Establish a connection to only one member of the replicaset (most 
     * probably the primary) */
    $mc = new MongoClient($rs["hosts"][0], array("replicaSet" => $rs["rsname"]));
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
}

/* Our predefined replicaset has 1 primary, 2 normal secondary, 1 passsive, and one arbiter
 * Since we only seeded with the primary, the arbiter won't show up, so we wind 
 * up with 4 connections */
$nfo = MongoClient::getConnections();
if (count($nfo) == 4) {
    echo "ok\n";
} else {
    echo "Failed\n";
    var_dump($nfo);
}

?>