<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	MySQLPDOTest::createTestTable($db);

	$default =  $db->getAttribute(PDO::ATTR_CASE);
	$known = array(
		PDO::CASE_LOWER => 'PDO::CASE_LOWER',
		PDO::CASE_UPPER => 'PDO::CASE_UPPER',
		PDO::CASE_NATURAL => 'PDO::CASE_NATURAL'
	);
	if (!isset($known[$default]))
		printf("[001] getAttribute(PDO::ATTR_CASE) returns unknown value '%s'\n",
			var_export($default, true));
	else
		var_dump($known[$default]);

	// lets see what the default is...
	if (!is_object($stmt = $db->query("SELECT id, id AS 'ID_UPPER', label FROM test ORDER BY id ASC LIMIT 2")))
		printf("[002] %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	var_dump($stmt->fetchAll(PDO::FETCH_BOTH));

	if (true !== $db->setAttribute(PDO::ATTR_CASE, PDO::CASE_LOWER))
		printf("[003] Cannot set PDO::ATTR_CASE = PDO::CASE_LOWER, %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	if (($tmp = $db->getAttribute(PDO::ATTR_CASE)) !== PDO::CASE_LOWER)
		printf("[004] getAttribute(PDO::ATTR_CASE) returns wrong value '%s'\n",
			var_export($tmp, true));

	if (true === $db->exec('ALTER TABLE test ADD MiXeD CHAR(1)'))
		printf("[005] Cannot add column %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	if (false === $db->exec('ALTER TABLE test ADD MYUPPER CHAR(1)'))
		printf("[006] Cannot add column %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	if (!is_object($stmt = $db->query("SELECT id, id AS 'ID_UPPER', label, MiXeD, MYUPPER FROM test ORDER BY id ASC LIMIT 2")))
		printf("[007] %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	var_dump($stmt->fetchAll(PDO::FETCH_BOTH));

	if (true !== $db->setAttribute(PDO::ATTR_CASE, PDO::CASE_UPPER))
		printf("[008] Cannot set PDO::ATTR_CASE = PDO::CASE_UPPER %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	if (($tmp = $db->getAttribute(PDO::ATTR_CASE)) !== PDO::CASE_UPPER)
		printf("[009] getAttribute(PDO::ATTR_CASE) returns wrong value '%s'\n",
			var_export($tmp, true));

	if (!is_object($stmt = $db->query("SELECT id, label, MiXeD, MYUPPER, MYUPPER AS 'lower' FROM test ORDER BY id ASC LIMIT 1")))
		printf("[010] %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	var_dump($stmt->fetchAll(PDO::FETCH_BOTH));

	if (true !== $db->setAttribute(PDO::ATTR_CASE, PDO::CASE_NATURAL))
		printf("[011] Cannot set PDO::ATTR_CASE = PDO::CASE_NATURAL %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	if (($tmp = $db->getAttribute(PDO::ATTR_CASE)) !== PDO::CASE_NATURAL)
		printf("[012] getAttribute(PDO::ATTR_CASE) returns wrong value '%s'\n",
			var_export($tmp, true));

	if (!is_object($stmt = $db->query("SELECT id, label, MiXeD, MYUPPER, id AS 'ID' FROM test ORDER BY id ASC LIMIT 1")))
		printf("[013] %s - %s\n",
			var_export($db->errorInfo(), true), var_export($db->errorCode(), true));

	var_dump($stmt->fetchAll(PDO::FETCH_BOTH));

	print "done!";
?>
<?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
MySQLPDOTest::dropTestTable();
?>