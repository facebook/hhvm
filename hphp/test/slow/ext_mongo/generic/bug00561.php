<?hh
require_once __DIR__."/../utils/server.inc";
<<__EntryPoint>> function main(): void {
MongoLog::setModule( MongoLog::PARSE );
MongoLog::setLevel( MongoLog::INFO );
set_error_handler(fun('foo')); function foo($a, $b, $c) { echo $b, "\n"; }

new MongoClient("mongodb://localhost", array( 'connect' => false ));
new MongoClient("mongodb://localhost/", array( 'connect' => false ));
new MongoClient("mongodb://localhost/x", array( 'connect' => false ));
new MongoClient("mongodb://localhost/?readPreference=PRIMARY", array( 'connect' => false ));
}
