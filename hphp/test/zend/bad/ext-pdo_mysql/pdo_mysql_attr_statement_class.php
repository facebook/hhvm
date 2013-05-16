<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	MySQLPDOTest::createTestTable($db);

	$default =  $db->getAttribute(PDO::ATTR_STATEMENT_CLASS);
	var_dump($default);

	if (false !== ($tmp = @$db->setAttribute(PDO::ATTR_STATEMENT_CLASS)))
		printf("[001] Expecting boolean/false got %s\n", var_export($tmp, true));

	if (false !== ($tmp = @$db->setAttribute(PDO::ATTR_STATEMENT_CLASS, 'foo')))
		printf("[002] Expecting boolean/false got %s\n", var_export($tmp, true));

	if (false !== ($tmp = @$db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('classname'))))
		printf("[003] Expecting boolean/false got %s\n", var_export($tmp, true));

	// unknown class
	if (false !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('classname', array()))))
		printf("[004] Expecting boolean/false got %s\n", var_export($tmp, true));

	// class not derived from PDOStatement
	class myclass {
		function __construct() {
			printf("myclass\n");
		}
	}
	if (false !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('myclass', array()))))
		printf("[005] Expecting boolean/false got %s\n", var_export($tmp, true));

	// public constructor not allowed
	class mystatement extends PDOStatement {
		public function __construct() {
			printf("mystatement\n");
		}
	}

	if (false !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('mystatement', array()))))
		printf("[006] Expecting boolean/false got %s\n", var_export($tmp, true));

	// ... but a public destructor is allowed
	class mystatement2 extends PDOStatement {
		public function __destruct() {
			printf("mystatement\n");
		}
	}

	if (true !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('mystatement2', array()))))
		printf("[007] Expecting boolean/true got %s\n", var_export($tmp, true));

	// private constructor
	class mystatement3 extends PDOStatement {
		private function __construct($msg) {
			printf("mystatement3\n");
			var_dump($msg);
		}
	}
	if (true !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('mystatement3', array('param1')))))
		printf("[008] Expecting boolean/true got %s\n", var_export($tmp, true));

	// private constructor
	class mystatement4 extends PDOStatement {
		private function __construct($msg) {
			printf("%s\n", get_class($this));
			var_dump($msg);
		}
	}
	if (true !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('mystatement4', array('param1')))))
		printf("[008] Expecting boolean/true got %s\n", var_export($tmp, true));

	var_dump($db->getAttribute(PDO::ATTR_STATEMENT_CLASS));
	$stmt = $db->query('SELECT id, label FROM test ORDER BY id ASC LIMIT 2');

	class mystatement5 extends mystatement4 {
		public function fetchAll($fetch_style = 1, $column_index = 1, $ctor_args = array()) {
			return "no data :)";
		}
	}

	if (true !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('mystatement5', array('mystatement5')))))
		printf("[009] Expecting boolean/true got %s\n", var_export($tmp, true));
	$stmt = $db->query('SELECT id, label FROM test ORDER BY id ASC LIMIT 2');
	var_dump($stmt->fetchAll());

	if (true !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('PDOStatement'))))
		printf("[010] Expecting boolean/true got %s\n", var_export($tmp, true));

	$stmt = $db->query('SELECT id, label FROM test ORDER BY id ASC LIMIT 1');
	var_dump($stmt->fetchAll());

	// Yes, this is a fatal error and I want it to fail.
	abstract class mystatement6 extends mystatement5 {
	}
	if (true !== ($tmp = $db->setAttribute(PDO::ATTR_STATEMENT_CLASS, array('mystatement6', array('mystatement6')))))
		printf("[011] Expecting boolean/true got %s\n", var_export($tmp, true));
	$stmt = $db->query('SELECT id, label FROM test ORDER BY id ASC LIMIT 1');

	print "done!";
?>