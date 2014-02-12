<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_stmt_close_table_1'; require('table.inc');

	if (!$stmt = mysqli_stmt_init($link))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	// Yes, amazing, eh? AFAIK a work around of a constructor bug...
	if (!is_null($tmp = mysqli_stmt_close($stmt)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_close_table_1"))
		printf("[005] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (true !== ($tmp = mysqli_stmt_close($stmt)))
		printf("[006] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = mysqli_stmt_close($stmt)))
		printf("[007] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!$stmt = mysqli_stmt_init($link))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, "INSERT INTO test_mysqli_stmt_close_table_1(id, label) VALUES (?, ?)"))
		printf("[009] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = $label = null;
	if (!mysqli_stmt_bind_param($stmt, "is", $id, $label))
		printf("[010] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = 100; $label = 'z';
	if (!mysqli_stmt_execute($stmt))
		printf("[011] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	mysqli_kill($link, mysqli_thread_id($link));

	if (true !== ($tmp = mysqli_stmt_close($stmt)))
		printf("[012] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	mysqli_close($link);

	$test_table_name = 'test_mysqli_stmt_close_table_1'; require('table.inc');
	if (!$stmt = mysqli_stmt_init($link))
		printf("[013] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_close_table_1"))
		printf("[014] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	$id = $label = null;
	if (!mysqli_stmt_bind_result($stmt, $id, $label))
		printf("[015] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	if (!mysqli_stmt_execute($stmt) || !mysqli_stmt_fetch($stmt))
		printf("[016] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));

	mysqli_kill($link, mysqli_thread_id($link));

	if (true !== ($tmp = mysqli_stmt_close($stmt)))
		printf("[017] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_close_table_1'; require_once("clean_table.inc");
?>