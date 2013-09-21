<?php

$db = new PDO('sqlite::memory:');
$db->exec('CREATE TABLE testing (id INTEGER , name VARCHAR)');
$db->exec('INSERT INTO testing VALUES(1, "php")');
$db->exec('INSERT INTO testing VALUES(2, "")');

$st = $db->query('SELECT * FROM testing');
$st->fetchAll(PDO::FETCH_FUNC, function($x, $y) use ($st) { var_dump($st); print "data: $x, $y\n"; });

$st = $db->query('SELECT name FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, 'strtoupper'));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, 'nothing'));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, ''));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, NULL));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, 1));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, array('self', 'foo')));

class foo { 
	public function foo($x) {
		return "--- $x ---";
	}
}
class bar extends foo {
	public function __construct($db) {
		$st = $db->query('SELECT * FROM testing');
		var_dump($st->fetchAll(PDO::FETCH_FUNC, array($this, 'parent::foo')));
	}
	
	static public function test($x, $y) {
		return $x .'---'. $y;
	}
	
	private function test2($x, $y) {
		return $x;
	}
	
	public function test3($x, $y) {
		return $x .'==='. $y;
	}
}

new bar($db);

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, array('bar', 'test')));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, array('bar', 'test2')));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, array('bar', 'test3')));

$st = $db->query('SELECT * FROM testing');
var_dump($st->fetchAll(PDO::FETCH_FUNC, array('bar', 'inexistent')));

?>