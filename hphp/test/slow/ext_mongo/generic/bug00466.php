<?php
MongoLog::setModule(MongoLog::ALL);
MongoLog::setLevel(MongoLog::ALL);
function foo($c, $m) { echo $m, "\n"; } set_error_handler('foo');
$m = new Mongo("mongodb://whisky:13000", array( "connect" => false, "replicaSet" => true ));
$m = new Mongo("mongodb://whisky:13000", array( "connect" => false, "replicaSet" => 'seta' ));
?>
