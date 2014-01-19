<?php
require_once __DIR__."/../utils/server.inc";
$host = standalone_hostname();
$port = standalone_port();
$user = "A";
$pass = "wrong password";
$db   = dbname();

try {
    $m = new mongo(sprintf("mongodb://%s:%s@%s:%d/%s", "", $pass, $host, $port, $db));
    echo "got a MongoClient object\n";
    $m->$db->collection->findOne();
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
try {
    $m = new mongo(sprintf("mongodb://%s:%s@%s:%d/%s", $user, "", $host, $port, $db));
    echo "got a MongoClient object\n";
    $m->$db->collection->findOne();
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
try {
    $m = new mongo(sprintf("mongodb://%s:%s@%s:%d/%s", $user . "bogus", $pass, $host, $port, $db));
    echo "got a MongoClient object\n";
    $m->$db->collection->findOne();
} catch (Exception $e) {
    echo $e->getMessage(), "\n";
}
?>