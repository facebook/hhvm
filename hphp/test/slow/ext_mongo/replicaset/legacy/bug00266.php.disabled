<?php
require_once __DIR__."/../../utils/server.inc";

$host = hostname();
$port = port();
$ip   = gethostbyname($host);

$host2  = hostname("STANDALONE");
$port2 = port("STANDALONE");
$ip2   = gethostbyname($host2);

$m = new Mongo("$ip:$port,$ip2:$port2", array("replicaSet" => true));
$coll = $m->selectCollection("phpunit","bug00266");
try {
    $coll->getIndexInfo();
    echo "I'm alive\n";
} catch(MongoCursorException $e) {
    var_dump($e->getMessage());
}
?>