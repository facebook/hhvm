<?php

class MyDB extends MongoDB {
	public function __construct() {}
}

$db = new MyDB;

try {
	MongoDBRef::get($db, array('$ref' => "", '$id' => 1));
} catch (MongoException $e) {
	var_dump($e->getCode());
	var_dump($e->getMessage());
}
?>
