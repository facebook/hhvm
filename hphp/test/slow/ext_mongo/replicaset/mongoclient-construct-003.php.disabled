<?php
require_once __DIR__."/../utils/server.inc";

$rs = MongoShellServer::getReplicasetInfo();
try {
    $mc = new MongoClient($rs["dsn"], array("readPreference" => MongoClient::RP_PRIMARY_PREFERRED));
    echo "ok\n";
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage());
}

?>