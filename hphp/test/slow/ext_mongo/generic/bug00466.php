<?hh
function foo($c, $m) { echo $m, "\n"; } <<__EntryPoint>>
function main_entry(): void {
  MongoLog::setModule(MongoLog::ALL);
  MongoLog::setLevel(MongoLog::ALL);
  set_error_handler(fun('foo'));
  $m = new Mongo("mongodb://whisky:13000", darray[ "connect" => false, "replicaSet" => true ]);
  $m = new Mongo("mongodb://whisky:13000", darray[ "connect" => false, "replicaSet" => 'seta' ]);
}
