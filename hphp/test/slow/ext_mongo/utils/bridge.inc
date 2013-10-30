<?php
require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $cfg = $server->getBridgeConfig();
    $server->close();
    if (!$cfg) {
        throw new Exception("No bridge configured");
    }
} catch(Exception $e) {
    die("skip " . $e->getMessage());
}


