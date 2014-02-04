<?php
	require_once("connect.inc");

	$tmp    = NULL;
	$link   = NULL;
	$test_table_name = 'test_mysqli_field_tell_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id FROM test_mysqli_field_tell_table_1 ORDER BY id LIMIT 1", MYSQLI_USE_RESULT)) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	var_dump(mysqli_field_tell($res));
	var_dump(mysqli_field_seek(1));
	var_dump(mysqli_field_tell($res));
	var_dump(mysqli_fetch_field($res));
	var_dump(mysqli_fetch_field($res));
	var_dump(mysqli_field_tell($res));
	var_dump(mysqli_field_seek($res, 2));
	var_dump(mysqli_field_tell($res));

	var_dump(mysqli_field_seek($res, -1));
	var_dump(mysqli_field_tell($res));

	var_dump(mysqli_field_seek($res, 0));
	var_dump(mysqli_field_tell($res));



	mysqli_free_result($res);

	var_dump(mysqli_field_tell($res));

	mysqli_close($link);

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_field_tell_table_1'; require_once("clean_table.inc");
?>