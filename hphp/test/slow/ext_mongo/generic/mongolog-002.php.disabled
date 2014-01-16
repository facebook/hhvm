<?php
require_once __DIR__."/../utils/server.inc";

function f($module, $log, $m) {
    var_dump($module, $log, $m);
}

var_dump(MongoLog::getCallback());

MongoLog::setModule(MongoLog::IO);
MongoLog::setLevel(MongoLog::FINE);
var_dump(MongoLog::setCallback("f"));


$mongo = mongo_standalone();


var_dump(MongoLog::getCallback());

echo "Selecting collection\n";
$coll = $mongo->selectCollection(dbname(), 'mongolog');
$coll->drop();
echo "Finished\n";
?>