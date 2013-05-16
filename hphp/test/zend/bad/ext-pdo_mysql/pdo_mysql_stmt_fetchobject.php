<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
$db = MySQLPDOTest::factory();
MySQLPDOTest::createTestTable($db);

try {

	$query = "SELECT id, '', NULL, \"\" FROM test ORDER BY id ASC LIMIT 3";
	$stmt = $db->prepare($query);

	class myclass {

		private $set_calls = 0;
		protected static $static_set_calls = 0;

		// NOTE: PDO does not care about protected
		protected $grp;

		// NOTE: PDO does not care about private and calls __construct() after __set()
		private function __construct($param1, $param2) {
			printf("myclass::__construct(%s, %s): %d / %d\n",
				$param1, $param2,
				self::$static_set_calls, $this->set_calls);
		}

		// NOTE: PDO will call __set() prior to calling __construct()
		public function __set($prop, $value) {
			$this->not_a_magic_one();
			printf("myclass::__set(%s, -%s-) %d\n",
				$prop, var_export($value, true), $this->set_calls, self::$static_set_calls);
			if ("" != $prop)
				$this->{$prop} = $value;
		}

		// NOTE: PDO can call regular methods prior to calling __construct()
		public function not_a_magic_one() {
			$this->set_calls++;
			self::$static_set_calls++;
		}

	}
	$stmt->execute();
	$rowno = 0;
	$rows[] = array();
	while (is_object($rows[] = $stmt->fetchObject('myclass', array($rowno++, $rowno))))
		;

	var_dump($rows[$rowno - 1]);

} catch (PDOException $e) {
	// we should never get here, we use warnings, but never trust a system...
	printf("[001] %s, [%s} %s\n",
		$e->getMessage(), $db->errorInfo(), implode(' ', $db->errorInfo()));
}

print "done!";
?><?php
require dirname(__FILE__) . '/mysql_pdo_test.inc';
MySQLPDOTest::dropTestTable();
?>