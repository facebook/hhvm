<?php
if (getenv('SKIP_SLOW_TESTS')) die('skip slow tests excluded by request');

require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $server->getReplicaSetConfig();
    $server->close();
} catch(Exception $e) {
    die("skip " . $e->getMessage());
}



