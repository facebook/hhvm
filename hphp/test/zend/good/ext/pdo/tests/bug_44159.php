<?php
$pdo = new PDO("sqlite:".__DIR__."/foo.db");

$attrs = array(PDO::ATTR_STATEMENT_CLASS, PDO::ATTR_STRINGIFY_FETCHES, PDO::NULL_TO_STRING);

foreach ($attrs as $attr) {
	var_dump($pdo->setAttribute($attr, NULL));
	var_dump($pdo->setAttribute($attr, 1));
	var_dump($pdo->setAttribute($attr, 'nonsense'));
}

@unlink(__DIR__."/foo.db");

?>