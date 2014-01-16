<?php
require_once __DIR__."/../utils/server.inc";
$dsn = MongoShellServer::getStandaloneInfo();
class MyMongoClient extends MongoClient {
}
class MyDB extends MongoDB {
    public function __construct() {}
}
 
$db = new MyDB;

$nss = array(
	'test', '.test', 'test.', '', '.', 'xx', 'xxx', 'a.b', 'db.foo', 'foo.file.fs',
);

foreach ($nss as $ns) {
	echo "NS: ", $ns, ": ";
	try {
		$c = new MongoCursor(new MyMongoClient($dsn), $ns);
		$c->hasNext();
		echo "OK\n";
	} catch (MongoException $e) {
		echo "FAIL\n";
		var_dump($e->getCode());
		var_dump($e->getMessage());
	}
	echo "\n";
}
?>