<?php
require_once __DIR__."/../../utils/server.inc";

$s = new MongoShellServer;
$s->addReplicasetUser("testuno", "username1", "password1");
$s->addReplicasetUser("testdos", "username2", "password2");
$cfg = $s->getReplicaSetConfig(true);

function queryMongoDB($connstr, $dbname, $collectionname, $fieldname)
{
    $m = new MongoClient($connstr, array('replicaSet' => true)); #just specify it as true instead of actual replica set. Either way the bug is reproduced.
    $db = $m->selectDB($dbname);
    $collection = $db->selectCollection($collectionname);
    $cursor = $collection->find();
    foreach ($cursor as $document) {
    }
    echo "I'm a survivor\n";
}

#MongoLog::setLevel(MongoLog::ALL); // all log levels
#MongoLog::setModule(MongoLog::ALL); // all parts of the driver

$dsn = "mongodb://username1:password1@" . $cfg["dsn"] . "/testuno";
queryMongoDB($dsn, "testuno", "foocollection", "fieldinfoocollection");
#Step 2: connect and query to bar db: This would fail randomly with message


$dsn = "mongodb://username2:password2@" . $cfg["dsn"] . "/testdos";
queryMongoDB($dsn, "testdos", "barcollection", "fieldinbarcollection.");
?>