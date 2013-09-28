<?php
require dirname(__FILE__) . '/config.inc';
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

function bug_45120($db) {

	$stmt = $db->prepare("SELECT 1 AS 'one'");
	if (true !== $stmt->execute())
		printf("[001] Execute has failed: %s\n", var_export($stmt->errorInfo(), true));

	$res = $stmt->fetch(PDO::FETCH_ASSOC);
	if ($res['one'] != 1)
		printf("[002] Wrong results: %s\n", var_export($res, true));

	if (true !== $stmt->execute())
		printf("[003] Execute has failed: %s\n", var_export($stmt->errorInfo(), true));

	$res = $stmt->fetch(PDO::FETCH_ASSOC);
	if ($res['one'] != 1)
		printf("[004] Wrong results: %s\n", var_export($res, true));

}

print "Emulated Prepared Statements\n";
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
bug_45120($db);

print "Native Prepared Statements\n";
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
bug_45120($db);

print "done!";
?>