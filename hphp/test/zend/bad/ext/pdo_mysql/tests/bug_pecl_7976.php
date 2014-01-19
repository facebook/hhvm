<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();

function bug_pecl_7976($db) {

	$db->exec('DROP PROCEDURE IF EXISTS p');
	$db->exec('CREATE PROCEDURE p() BEGIN SELECT "1" AS _one; END;');

	$stmt = $db->query('CALL p()');
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
	$stmt->closeCursor();

	$stmt = $db->query('CALL p()');
	var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
	$stmt->closeCursor();

}

printf("Emulated...\n");
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
bug_pecl_7976($db);

printf("Native...\n");
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
bug_pecl_7976($db);

print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec('DROP PROCEDURE IF EXISTS p');
?>