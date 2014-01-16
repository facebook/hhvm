<?php
require_once __DIR__."/../utils/server.inc";
try {
    $host = hostname();
    $port = standalone_port();
    $user = "wrong username";
    $pass = "failed password";
    $db   = dbname();

    $m = new mongo(sprintf("mongodb://%s:%s@%s:%d/%s", $user, $pass, $host, $port, $db));
    echo "Have mongo client object\n";
    $m->$db->collection->findOne();
} catch (Exception $e) {
	echo $e->getMessage(), "\n";
}
?>