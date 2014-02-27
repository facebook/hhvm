<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$stmt = @new mysqli_stmt($link);
	$test_table_name = 'test_mysqli_stmt_insert_id_table_1'; require('table.inc');

	$stmt = mysqli_stmt_init($link);
	if (!mysqli_stmt_prepare($stmt, "SELECT id, label FROM test_mysqli_stmt_insert_id_table_1 ORDER BY id LIMIT 1") ||
		!mysqli_stmt_execute($stmt)) {
		printf("[004] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	if (0 !== ($tmp = mysqli_stmt_insert_id($stmt)))
		printf("[005] Expecting int/0, got %s/%s\n", gettype($tmp), $tmp);
	mysqli_stmt_close($stmt);

	// no auto_increment column
	$stmt = mysqli_stmt_init($link);
	if (!mysqli_stmt_prepare($stmt, "INSERT INTO test_mysqli_stmt_insert_id_table_1(id, label) VALUES (100, 'a')") ||
		!mysqli_stmt_execute($stmt)) {
		printf("[006] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}

	if (0 !== ($tmp = mysqli_stmt_insert_id($stmt)))
		printf("[007] Expecting int/0, got %s/%s\n", gettype($tmp), $tmp);

	if (mysqli_get_server_version($link) > 50000 &&
		(!mysqli_stmt_prepare($stmt, "ALTER TABLE test_mysqli_stmt_insert_id_table_1 MODIFY id INT NOT NULL AUTO_INCREMENT") ||
		!mysqli_stmt_execute($stmt))) {
		printf("[008] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	} else if (mysqli_get_server_version($link) < 50000){
		mysqli_query($link, "ALTER TABLE test_mysqli_stmt_insert_id_table_1 MODIFY id INT NOT NULL AUTO_INCREMENT");
	}

	if (!mysqli_stmt_prepare($stmt, "INSERT INTO test_mysqli_stmt_insert_id_table_1(label) VALUES ('a')") ||
		!mysqli_stmt_execute($stmt)) {
		printf("[009] [%d] %s\n", mysqli_stmt_errno($stmt), mysqli_stmt_error($stmt));
	}
	if (0 === ($tmp = mysqli_stmt_insert_id($stmt)))
		printf("[010] Expecting int/any non zero, got %s/%s\n", gettype($tmp), $tmp);
	mysqli_stmt_close($stmt);

	mysqli_close($link);

	var_dump(mysqli_stmt_insert_id($stmt));

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_stmt_insert_id_table_1'; require_once("clean_table.inc");
?>