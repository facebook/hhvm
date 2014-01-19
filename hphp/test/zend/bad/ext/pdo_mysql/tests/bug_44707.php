<?php
require dirname(__FILE__) . '/config.inc';
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

function bug_44707($db) {

	$db->exec('DROP TABLE IF EXISTS test');
	$db->exec('CREATE TABLE test(id INT, mybool TINYINT)');

	$id = 1;
	$mybool = false;
	var_dump($mybool);

	$stmt = $db->prepare('INSERT INTO test(id, mybool) VALUES (?, ?)');
	$stmt->bindParam(1, $id);
	// From MySQL 4.1 on boolean and TINYINT don't match! INSERT will fail.
	// Versions prior to 4.1 have a weak test and will accept this.
	$stmt->bindParam(2, $mybool, PDO::PARAM_BOOL);
	var_dump($mybool);

	$stmt->execute();
	var_dump($mybool);

	$stmt = $db->query('SELECT * FROM test');
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

	$stmt = $db->prepare('INSERT INTO test(id, mybool) VALUES (?, ?)');
	$stmt->bindParam(1, $id);
	// INT and integer work well together
	$stmt->bindParam(2, $mybool, PDO::PARAM_INT);
	$stmt->execute();

	$stmt = $db->query('SELECT * FROM test');
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));

}


/*
// This is beyond the control of the driver... - the driver never gets in touch with bound values
print "Emulated Prepared Statements\n";
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
bug_44707($db);
*/

print "Native Prepared Statements\n";
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
bug_44707($db);

print "done!";
?>