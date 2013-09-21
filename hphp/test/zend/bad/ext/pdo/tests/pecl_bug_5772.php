<?php
if (getenv('REDIR_TEST_DIR') === false) putenv('REDIR_TEST_DIR='.dirname(__FILE__) . '/../../pdo/tests/');
require_once getenv('REDIR_TEST_DIR') . 'pdo_test.inc';
$db = PDOTest::factory();

$db->exec("CREATE TABLE test (id int NOT NULL, PRIMARY KEY (id))");
$db->exec("INSERT INTO test (id) VALUES (1)");

function heLLO($row) {
	return $row;
}

foreach ($db->query("SELECT * FROM test")->fetchAll(PDO::FETCH_FUNC, 'heLLO') as $row) {
	var_dump($row);
}
