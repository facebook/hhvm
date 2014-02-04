<?php
	$test_table_name = 'test_mysqli_stmt_bind_result_references_table_1'; require('table.inc');

	$stmt = mysqli_stmt_init($link);
	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_bind_result_references_table_1 ORDER BY id LIMIT 1"))
		printf("[001] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_bind_result_references_table_1 ORDER BY id LIMIT 1"))
		printf("[001] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));


	print "plain vanilla...\n";
	unset($id); unset($label);
	$id = $label = null;
	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label)))
		printf("[002] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[003] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($id);
	var_dump($label);


	print "reference, one level...\n";
	unset($id); unset($id_ref); unset($label); unset($label_ref);
	$id = null;
	$id_ref = &$id;
	$label = null;
	$label_ref = &$label;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id_ref, $label_ref)))
		printf("[004] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[005] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	var_dump($id_ref);
	var_dump($id);
	var_dump($label_ref);
	var_dump($label);


	print "reference, two levels...\n";
	unset($id); unset($id_ref); unset($id_ref_ref); unset($label); unset($label_ref); unset($label_ref_ref);
	$id = null;
	$id_ref = &$id;
	$id_ref_ref = &$id_ref;
	$label = null;
	$label_ref = &$label;
	$label_ref_ref = &$label_ref;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id_ref_ref, $label_ref_ref)))
		printf("[006] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[007] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	var_dump($id_ref_ref);
	var_dump($id_ref);
	var_dump($id);
	var_dump($label_ref_ref);
	var_dump($label_ref);
	var_dump($label);

	print "reference, \$GLOBALS...\n";
	unset($id); unset($id_ref); unset($label); unset($label_ref);
	$id = 100;
	$id_ref = &$GLOBALS['id'];
	$label = null;
	$label_ref = &$GLOBALS['label'];

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id_ref, $label_ref)))
		printf("[008] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[009] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	var_dump($id_ref);
	var_dump($id);
	var_dump($label_ref);
	var_dump($label);

	print "reference, same target...\n";
	$id = null;
	$label = &$id;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label)))
		printf("[010] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[011] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($id);
	var_dump($label);

	print "reference, simple object...\n";
	unset($obj);
	$obj = new stdClass();
	$obj->id = null;
	$obj->label = null;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $obj->id, $obj->label)))
		printf("[012] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[013] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($obj->id);
	var_dump($obj->label);


	print "reference, simple object w reference...\n";
	unset($id); unset($label); unset($obj);
	$obj = new stdClass();
	$obj->id = null;
	$obj->label = null;
	$id = &$obj->id;
	$label = &$obj->label;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label)))
		printf("[012] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[013] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($obj->id);
	var_dump($obj->label);

	print "reference, simple object w reference, change after bind...\n";
	unset($id); unset($label); unset($obj);
	$obj = new stdClass();
	$obj->id = null;
	$obj->label = null;
	$id = &$obj->id;
	$label = &$obj->label;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label)))
		printf("[012] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	$label = &$obj->id;
	$id = null;

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[013] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($obj->id);
	var_dump($id);
	var_dump($obj->label);
	var_dump($label);

	print "reference, one level, change after bind...\n";
	unset($id); unset($label); unset($id_ref); unset($label_ref);
	$id = null;
	$id_ref = &$id;
	$label = null;
	$label_ref = &$label;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id_ref, $label_ref)))
		printf("[014] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	$id_ref = 1;
	$label_ref = 1;

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[015] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	var_dump($id_ref);
	var_dump($id);
	var_dump($label_ref);
	var_dump($label);

	print "reference, circle...\n";
	unset($id); unset($label_a); unset($label_b);
	$id = null;
	$label_a = &$label_b;
	$label_b = &$label_a;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label_a)))
		printf("[016] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);
	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[017] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($id);
	var_dump($label_a);
	var_dump($label_b);

	print "reference, object, forward declaration...\n";
	unset($bar); unset($id); unset($label_ref);
	class foo {
		public $foo;
		public function foo() {
			$this->foo = &$this->bar;
		}
	}
	class bar extends foo {
		public $bar = null;
	}
	$bar = new bar();
	$id = null;
	$label_ref = &$bar->bar;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label_ref)))
		printf("[018] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);
	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[019] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	var_dump($id);
	var_dump($bar);
	var_dump($label_ref);

	print "references, object, private...\n";
	unset($bar); unset($id); unset($label);
	class mega_bar extends bar {
		private $id;
		public $id_ref;
		public function mega_bar() {
			$this->foo();
			$this->id_ref = &$this->id;
		}
	}
	$bar = new mega_bar();
	$id = &$bar->id_ref;
	$label = &$bar->foo;

	if (true !== ($tmp = mysqli_stmt_bind_result($stmt, $id, $label)))
		printf("[020] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);
	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt) || mysqli_stmt_fetch($stmt))
		printf("[021] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	var_dump($id);
	var_dump($label);
	var_dump($bar);

	mysqli_stmt_close($stmt);
	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_bind_result_references_table_1'; require_once("clean_table.inc");
?>