<?php
require dirname(__FILE__) . '/../../../ext/pdo/tests/pdo_test.inc';
$db = PDOTest::test_factory(dirname(__FILE__) . '/common.phpt');

$stmt = $db->prepare("select 1 as attr");
for ($i = 0; $i < 10; $i++) {
	$stmt->setFetchMode(PDO::FETCH_INTO, new stdClass);
}

print "ok\n";

?>