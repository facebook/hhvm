<?php
require_once __DIR__."/../../utils/server.inc";

$dsn = MongoShellServer::getBridgeInfo();
$m = new MongoClient($dsn);
$c = $m->selectDB(dbname())->selectCollection("collection");

try {
    $foo = array("foo" => time());
    $result = $c->insert($foo, array("safe" => true, "timeout" => 1));
} catch(Exception $e) {
    var_dump(get_class($e), $e->getMessage(), $e->getCode());
    var_dump($foo);
}
try {
    $foo = array("foo" => "bar");
    $c->insert($foo, array("safe" => true));
    $result = $c->findOne(array("_id" => $foo["_id"]));
    var_dump($result);
} catch(Exception $e) {
    printf("FAILED %s: %s\n", get_class($e), $e->getMessage());
}
?>
===DONE===