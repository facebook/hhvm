<?php
require_once __DIR__."/../../utils/server.inc";

$host = hostname();
$port = port();
ini_set("mongo.default_host", $host);
ini_set("mongo.default_port", $port);

$opts = array(
    "replicaSet" => rsname(),
    "timeout"    => "42",
    "username"   => array("something"),
    "password"   => array("else"),
    "connect"    => "0",
);
$m = new Mongo(null, $opts);
var_dump($opts, $m);
echo "All done\n";
?>