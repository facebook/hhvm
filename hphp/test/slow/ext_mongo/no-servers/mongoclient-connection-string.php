<?hh
<<__EntryPoint>> function main(): void {
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::ALL);



echo "First one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock/?replicaSet=foo", darray["connect" => 0]);
} catch(Exception $e) {}


echo "\nSecond one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock/databasename?replicaSet=foo", darray["connect" => 0]);
} catch(Exception $e) {}


echo "\nThird one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock", darray["connect" => 0]);
} catch(Exception $e) {}


echo "\nForth one\n";
try {
    $m = new Mongo("mongodb:///tmp/mongodb-27017.sock/databasename", darray["connect" => 0]);
} catch(Exception $e) {}
}
