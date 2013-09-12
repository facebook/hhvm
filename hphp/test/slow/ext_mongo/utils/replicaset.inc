<?php
require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $server->getReplicaSetConfig();
    $server->close();
    if (isset($needs)) {
        $serverversion = $server->getServerVersion("REPLICASET");
        if (version_compare($serverversion, $needs, "lt")) {
            die("skip needs MongoDB $needs, but is running $serverversion");
        }
    }
} catch(Exception $e) {
    exit("skip " . $e->getMessage());
}

