<?php
require_once "tests/utils/server.inc";

try {
    $server = new MongoShellServer;
    $server->getUnixStandaloneConfig();
    if (isset($needs)) {
        $serverversion = $server->getServerVersion("STANDALONE");
        if (version_compare($serverversion, $needs, "lt")) {
            throw new Exception("needs MongoDB $needs, but is running $serverversion");
        }
    }
    $server->close();
} catch(Exception $e) {
    die("skip " . $e->getMessage());
}

