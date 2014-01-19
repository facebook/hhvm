<?php
require_once __DIR__."/../utils/server.inc";

$host = standalone_hostname();
$port = standalone_port();
ini_set("mongo.default_host", $host);
ini_set("mongo.default_port", $port);

$m = new Mongo;
foreach($m->getHosts() as $s) {
    if ($s["host"] == $host) {
        echo "Connected to correct host\n";
    }
    if ($s["port"] == $port) {
        echo "Connected to correct port\n";
    }
}
echo "All done\n";
?>