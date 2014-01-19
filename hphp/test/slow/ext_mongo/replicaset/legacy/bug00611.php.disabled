<?php
require_once __DIR__."/../../utils/server.inc";

$hostname = hostname();
$opts = array(
    "readPreference"     => MongoClient::RP_PRIMARY_PREFERRED,
    "readPreferenceTags" => "dc:no;dc:eu;",
    "replicaSet"         => rsname(),
);
try {
    $m = new MongoClient($hostname, $opts);
} catch(MongoConnectionException $e) {
    echo $e->getMessage(), "\n";
}
echo "I'm alive\n";
?>