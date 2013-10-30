<?php
require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $cfg = $server->getStandaloneConfig(true);
    $server->close();
} catch(Exception $e) {
    die("skip " . $e->getMessage());
}


