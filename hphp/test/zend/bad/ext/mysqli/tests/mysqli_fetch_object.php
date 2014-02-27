<?php
	include_once("connect.inc");

	set_error_handler('handle_catchable_fatal');

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_fetch_object_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_object_table_1 AS TEST ORDER BY id LIMIT 5")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	$obj = mysqli_fetch_object($res);
	if (($obj->ID !== "1") || ($obj->label !== "a") || (get_class($obj) != 'stdClass')) {
		printf("[004] Object seems wrong. [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		var_dump($obj);
	}

	class mysqli_fetch_object_test {

		public $a = null;
		public $b = null;

		public function toString() {
			var_dump($this);
		}
	}

	$obj = mysqli_fetch_object($res, 'mysqli_fetch_object_test');
	if (($obj->ID !== "2") || ($obj->label !== "b") || ($obj->a !== NULL) || ($obj->b !== NULL) || (get_class($obj) != 'mysqli_fetch_object_test')) {
		printf("[005] Object seems wrong. [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		var_dump($obj);
	}



	class mysqli_fetch_object_construct extends mysqli_fetch_object_test {

		public function __construct($a, $b) {
			$this->a = $a;
			$this->b = $b;
		}

	}

	$obj = mysqli_fetch_object($res, 'mysqli_fetch_object_construct', array());

	if (($obj->ID !== "3") || ($obj->label !== "c") || ($obj->a !== NULL) || ($obj->b !== NULL) || (get_class($obj) != 'mysqli_fetch_object_construct')) {
		printf("[006] Object seems wrong. [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		var_dump($obj);
	}

	$obj = mysqli_fetch_object($res, 'mysqli_fetch_object_construct', array('a'));
	if (($obj->ID !== "4") || ($obj->label !== "d") || ($obj->a !== 'a') || ($obj->b !== NULL) || (get_class($obj) != 'mysqli_fetch_object_construct')) {
		printf("[007] Object seems wrong. [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		var_dump($obj);
	}

	$obj = mysqli_fetch_object($res, 'mysqli_fetch_object_construct', array('a', 'b'));
	if (($obj->ID !== "5") || ($obj->label !== "e") || ($obj->a !== 'a') || ($obj->b !== 'b') || (get_class($obj) != 'mysqli_fetch_object_construct')) {
		printf("[008] Object seems wrong. [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		var_dump($obj);
	}

	var_dump(mysqli_fetch_object($res, 'mysqli_fetch_object_construct', array('a', 'b', 'c')));
	var_dump(mysqli_fetch_object($res));

	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_object_table_1 AS TEST")) {
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	mysqli_free_result($res);
	var_dump(mysqli_fetch_object($res));

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_object_table_1 AS TEST ORDER BY id LIMIT 5"))
			printf("[010] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	/*
	TODO
	I'm using the procedural interface, this should not throw an exception.
	Also, I did not ask to get exceptions using the mysqli_options()
	*/
	try {
		if (false !== ($obj = @mysqli_fetch_object($res, 'mysqli_fetch_object_construct', 'a')))
			printf("[011] Should have failed\n");
	} catch (Exception $e) {
		printf("%s\n", $e->getMessage());
	}

	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_object_table_1 AS TEST ORDER BY id LIMIT 5"))
		printf("[012] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	class mysqli_fetch_object_private_constructor extends mysqli_fetch_object_test {

		private function __construct($a, $b) {
			$this->a = $a;
			$this->b = $b;
		}
	}
	/*
	TODO
	I think we should bail out here. The following line will give a Fatal error: Call to private ... from invalid context
	var_dump($obj = new mysqli_fetch_object_private_constructor(1, 2));
	This does not fail.
	*/
	$obj = mysqli_fetch_object($res, 'mysqli_fetch_object_private_constructor', array('a', 'b'));
	mysqli_free_result($res);

	// Fatal error, script execution will end
	var_dump(mysqli_fetch_object($res, 'this_class_does_not_exist'));


	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_object_table_1'; require_once("clean_table.inc");
?>