<?php
require_once "tests/utils/server.inc";

$server = new MongoShellServer;
$server->restartMaster();
$server->close();

