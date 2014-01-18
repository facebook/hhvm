<?php
require_once __DIR__."/../../utils/server.inc";
MongoLog::setLevel(MongoLog::WARNING);
MongoLog::setModule(MongoLog::CON);
$dsn = MongoShellServer::getStandaloneInfo();
try {
    $m = new Mongo($dsn, array("replicaSet" => true));
    echo "Have mongo object\n";
    $m->foo->bar->findOne();
} catch(MongoConnectionException $e) {
    var_dump($e->getMessage());
}
echo "I'm alive!\n";
?>
==DONE==