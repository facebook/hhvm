<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;

	$test_table_name = 'test_mysqli_fetch_object_no_constructor_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test_mysqli_fetch_object_no_constructor_table_1 AS TEST ORDER BY id LIMIT 5")) {
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	class mysqli_fetch_object_test {

		public $a = null;
		public $b = null;

		public function toString() {
			var_dump($this);
		}
	}

	printf("No exception with PHP:\n");
	var_dump($obj = new mysqli_fetch_object_test(1, 2));

	printf("\nException with mysqli. Note that at all other places we throws errors but no exceptions unless the error mode has been changed:\n");
	try {
		var_dump($obj = mysqli_fetch_object($res, 'mysqli_fetch_object_test', array(1, 2)));
	} catch (Exception $e) {
		printf("Exception: %s\n", $e->getMessage());
	}

	printf("\nFatal error with PHP (but no exception!):\n");
	var_dump($obj->mysqli_fetch_object_test(1, 2));

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_fetch_object_no_constructor_table_1'; require_once("clean_table.inc");
?>