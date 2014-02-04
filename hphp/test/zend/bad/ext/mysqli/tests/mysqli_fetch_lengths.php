<?php
	require_once("connect.inc");

	if (!$mysqli = new mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[001] Cannot connect\n");

	if (!is_null($tmp = @mysqli_fetch_lengths()))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @mysqli_fetch_lengths($link)))
		printf("[002] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$test_table_name = 'test_mysqli_fetch_lengths_table_1'; require('table.inc');
	if (!$res = mysqli_query($link, "SELECT id, label FROM test_mysqli_fetch_lengths_table_1 ORDER BY id LIMIT 1")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}

	var_dump(mysqli_fetch_lengths($res));
	while ($row = mysqli_fetch_assoc($res))
		var_dump(mysqli_fetch_lengths($res));
	var_dump(mysqli_fetch_lengths($res));

	mysqli_free_result($res);

	var_dump(mysqli_fetch_lengths($res));

	mysqli_close($link);
	print "done!";
?>
<?php
	require_once("clean_table.inc");
?>