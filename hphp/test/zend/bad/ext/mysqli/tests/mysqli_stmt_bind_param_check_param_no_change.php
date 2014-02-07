<?php
	$test_table_name = 'test_mysqli_stmt_bind_param_check_param_no_change_table_1'; require('table.inc');
	$link->set_charset('latin1');

	class foo {
	  // @var $bar string
	  public $bar;
	}

	$foo = new foo;
	$foo->bar = "фубар";

	echo "Test 1:\n";
	$stmt = $link->prepare("SELECT ? FOO");
	var_dump($foo); // here you can see the bar member var beeing a string
	$stmt->bind_param("s", $foo->bar);
	var_dump($foo); // this will show $foo->bar beeing a reference string
	$stmt->bind_result($one);
	$stmt->execute();
	$stmt->fetch();
	$stmt->free_result();
	echo("$one\n\n");

	// it is getting worse. Binding the same var twice with different
	// types you can get unexpected results (e.g. binary trash for the
	// string and misc data for the integer. See next 2 tests.

	echo "Test 2:\n";
	$stmt = $link->prepare("SELECT ? FOO, ? BAR");
	var_dump($foo);
	$stmt->bind_param("si", $foo->bar, $foo->bar);
	echo "---\n";
	var_dump($foo);
	echo "---\n";
	$stmt->execute();
	var_dump($foo);
	echo "---\n";
	$stmt->bind_result($one, $two);
	$stmt->fetch();
	$stmt->free_result();
	echo("$one - $two\n\n");


	echo "Test 3:\n";
	$stmt = $link->prepare("SELECT ? FOO, ? BAR");
	var_dump($foo);
	$stmt->bind_param("is", $foo->bar, $foo->bar);
	var_dump($foo);
	$stmt->bind_result($one, $two);
	$stmt->execute();
	$stmt->fetch();
	$stmt->free_result();
	echo("$one - $two\n\n");
	echo "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_bind_param_check_param_no_change_table_1'; require_once("clean_table.inc");
?>