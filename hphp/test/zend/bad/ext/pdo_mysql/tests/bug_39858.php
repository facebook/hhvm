<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();
$db->setAttribute(PDO::ATTR_STRINGIFY_FETCHES, true);

function bug_39858($db) {

	$db->exec("DROP PROCEDURE IF EXISTS p");
	$db->exec("
		CREATE PROCEDURE p()
			NOT DETERMINISTIC
			CONTAINS SQL
			SQL SECURITY DEFINER
			COMMENT ''
		BEGIN
			SELECT 2 * 2;
		END;");

	$stmt = $db->prepare("CALL p()");
	$stmt->execute();
	do {
		var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
	} while ($stmt->nextRowset());

	$stmt = $db->prepare("CALL p()");
	$stmt->execute();
	do {
		var_dump($stmt->fetchAll(PDO::FETCH_ASSOC));
	} while ($stmt->nextRowset());
	$stmt->closeCursor();

}

printf("Emulated Prepared Statements...\n");
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
bug_39858($db);

printf("Native Prepared Statements...\n");
$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
bug_39858($db);

print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
$db = MySQLPDOTest::factory();
$db->exec("DROP PROCEDURE IF EXISTS p");
?>