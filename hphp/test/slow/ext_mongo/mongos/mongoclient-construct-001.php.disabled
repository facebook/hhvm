<?php
require_once __DIR__."/../utils/server.inc";

$cfg = MongoShellServer::getShardInfo();
try {
    $mc = new MongoClient($cfg[0]);
    echo "ok\n";
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
}

?>