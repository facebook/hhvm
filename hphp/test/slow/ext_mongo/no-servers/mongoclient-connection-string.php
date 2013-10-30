<?php

MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::ALL);



echo "First one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock/?replicaSet=foo", array("connect" => 0));
} catch(Exception $e) {}


echo "\nSecond one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock/databasename?replicaSet=foo", array("connect" => 0));
} catch(Exception $e) {}


echo "\nThird one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock", array("connect" => 0));
} catch(Exception $e) {}


echo "\nForth one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock/databasename", array("connect" => 0));
} catch(Exception $e) {}
?>
