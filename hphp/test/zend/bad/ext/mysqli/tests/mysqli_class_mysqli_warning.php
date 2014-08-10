<?php
	require('connect.inc');

	$warning = new mysqli_warning();
	$warning = new mysqli_warning(null);
	$warning = new mysqli_warning(null, null);

	$mysqli = new mysqli();
	$warning = new mysqli_warning($mysqli);

	$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket);
	$stmt = new mysqli_stmt($mysqli);
	$warning = new mysqli_warning($stmt);

	$stmt = $mysqli->stmt_init();
	$warning = new mysqli_warning($stmt);

	$obj = new stdClass();
	$warning = new mysqli_warning($obj);

	include("table.inc");
	$mysqli = new my_mysqli($host, $user, $passwd, $db, $port, $socket);
	$res = $mysqli->query('INSERT INTO test(id, label) VALUES (1, "zz")');
	$warning = mysqli_get_warnings($mysqli);

	printf("Parent class:\n");
	var_dump(get_parent_class($warning));

	printf("\nMethods:\n");
	$methods = get_class_methods($warning);
	$expected_methods = array(
		'next'                      => true,
	);

	foreach ($methods as $k => $method) {
		if (isset($expected_methods[$method])) {
			unset($methods[$k]);
			unset($expected_methods[$method]);
		}
	}
	if (!empty($methods)) {
		printf("Dumping list of unexpected methods.\n");
		var_dump($methods);
	}
	if (!empty($expected_methods)) {
		printf("Dumping list of missing methods.\n");
		var_dump($expected_methods);
	}
	if (empty($methods) && empty($expected_methods))
		printf("ok\n");

	printf("\nClass variables:\n");
	$variables = get_class_vars(get_class($mysqli));
	sort($variables);
	foreach ($variables as $k => $var)
		printf("%s\n", $var);

	printf("\nObject variables:\n");
	$variables = get_object_vars($mysqli);
	foreach ($variables as $k => $var)
		printf("%s\n", $var);

	printf("\nMagic, magic properties:\n");

	assert('' === $warning->message);
	printf("warning->message = '%s'\n", $warning->message);

	assert('' === $warning->sqlstate);
	printf("warning->sqlstate= '%s'\n", $warning->sqlstate);

	assert(0 === $warning->errno);
	printf("warning->errno = '%s'\n", $warning->errno);

	printf("\nAccess to undefined properties:\n");
	printf("warning->unknown = '%s'\n", @$warning->unknown);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_class_mysqli_warning_table_1'; require_once("clean_table.inc");
?>