<?php
	require_once("connect.inc");
	$test_table_name = 'test_mysqli_stmt_fetch_fields_win32_unicode_table_1'; require_once('table.inc');

	$bind_res = $id = null;
	if (!($stmt = mysqli_stmt_init($link)) ||
		!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_fetch_fields_win32_unicode_table_1") ||
		!mysqli_stmt_execute($stmt) ||
		!($result = mysqli_stmt_result_metadata($stmt)) ||
		!mysqli_stmt_bind_result($stmt, $id, $bind_res) ||
		!($fields = mysqli_fetch_fields($result))) {
		printf("FAIL 1\n");
	}
	while (mysqli_stmt_fetch($stmt)) {
		;
	}
	mysqli_free_result($result);
	mysqli_stmt_close($stmt);

	if (!($stmt = mysqli_stmt_init($link)) ||
		!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_fetch_fields_win32_unicode_table_1") ||
		!mysqli_stmt_execute($stmt) ||
		!($result = mysqli_stmt_result_metadata($stmt)) ||
		!mysqli_stmt_bind_result($stmt, $id, $bind_res)) {
		printf("FAIL 2\n");
	}
	print "OK: 1\n";
	if (!($fields = mysqli_fetch_fields($result)))
		printf("Aua 3\n");
	print "OK: 2\n";
	while (mysqli_stmt_fetch($stmt)) {
		;
	}
	mysqli_free_result($result);
	mysqli_stmt_close($stmt);

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_fetch_fields_win32_unicode_table_1'; require_once("clean_table.inc");
?>