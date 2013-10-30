<?php
include __DIR__.'/../utils/server.inc';

printlogs(MongoLog::ALL, MongoLog::ALL, '/^- Found option \'w\'/');
$formats = array(
	'w=',
	'w=0',
	'w=1',
	'w=1-',
	'w=fasdfads',
	'w=873253',
	'w=-1',
	'w=majority',
	'w=allDCs',
	'w=3.141592654',
);

foreach($formats as $format) {
	try {
		$m = new MongoClient('mongodb://localhost/?' . $format, array('connect' => false ) );
	} catch (MongoConnectionException $e) {
		var_dump($e->getCode());
		var_dump($e->getMessage());
	}
	echo "\n";
}
?>
