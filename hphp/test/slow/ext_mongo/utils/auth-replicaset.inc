<?php
require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $retval = $server->getReplicaSetConfig(true);
    $server->close();
} catch(Exception $e) {
    exit("skip " . $e->getMessage());
}

