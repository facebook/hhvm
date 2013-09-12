<?php
require_once "tests/utils/server.inc";

$server = new MongoShellServer;
$server->setMaintenanceForSecondaries(false);
$server->close();

