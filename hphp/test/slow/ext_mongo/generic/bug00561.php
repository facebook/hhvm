<?php
require_once __DIR__."/../utils/server.inc";

MongoLog::setModule( MongoLog::PARSE );
MongoLog::setLevel( MongoLog::INFO );
set_error_handler( 'foo' ); function foo($a, $b, $c) { echo $b, "\n"; }

new MongoClient("mongodb://localhost", array( 'connect' => false ));
new MongoClient("mongodb://localhost/", array( 'connect' => false ));
new MongoClient("mongodb://localhost/x", array( 'connect' => false ));
new MongoClient("mongodb://localhost/?readPreference=PRIMARY", array( 'connect' => false ));
?>
