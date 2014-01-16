<?php
require_once __DIR__."/../utils/server.inc";

mongo_standalone("", false);
mongo_standalone("phpunit", false);
mongo_standalone("bar/baz", false);
mongo_standalone("/", true);

if (isset($_ENV["MONGO_SERVER"]) && $_ENV["MONGO_SERVER"] == "REPLICASET")  {
    new mongo("$REPLICASET_PRIMARY:$REPLICASET_PRIMARY_PORT,$REPLICASET_SECONDARY:$REPLICASET_SECONDARY_PORT");
}
?>