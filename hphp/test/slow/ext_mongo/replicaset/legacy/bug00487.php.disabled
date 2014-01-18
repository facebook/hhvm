<?php
require_once __DIR__."/../../utils/server.inc";
$cfg = MongoShellServer::getReplicasetInfo();
$m = new Mongo($cfg["hosts"][2]);

try {
    $c = $m->selectDb(dbname())->test;
    var_dump($c->findOne());
} catch(Exception $e) {
    var_dump($e->getMessage());
}


$retval = $m->setSlaveOkay(true);
var_dump($retval);

$c = $m->selectDb(dbname())->test;
// We don't care about the data, just the fact we don't get an exception here
$c->findOne();
?>
===DONE===