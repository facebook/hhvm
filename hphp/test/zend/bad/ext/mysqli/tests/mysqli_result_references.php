<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_result_references_table_1'; require_once('table.inc');

	$references = array();

	if (!(mysqli_real_query($link, "SELECT id, label FROM test_mysqli_result_references_table_1 ORDER BY id ASC LIMIT 2")) ||
			!($res = mysqli_store_result($link)))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$idx = 0;
	while ($row = mysqli_fetch_assoc($res)) {
		/* mysqlnd: force seperation - create copies */
		$references[$idx] = array(
			'id' 		=> &$row['id'],
			'label'	=> $row['label'] . '');
		$references[$idx++]['id'] += 0;
	}

	mysqli_close($link);

	mysqli_data_seek($res, 0);
	while ($row = mysqli_fetch_assoc($res)) {
		/* mysqlnd: force seperation - create copies */
		$references[$idx] = array(
			'id' 		=> &$row['id'],
			'label'	=> $row['label'] . '');
		$references[$idx++]['id'] += 0;
	}

	mysqli_free_result($res);

	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	if (!(mysqli_real_query($link, "SELECT id, label FROM test_mysqli_result_references_table_1 ORDER BY id ASC LIMIT 2")) ||
			!($res = mysqli_use_result($link)))
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	while ($row = mysqli_fetch_assoc($res)) {
		/* mysqlnd: force seperation - create copies*/
		$references[$idx] = array(
			'id' 		=> &$row['id'],
			'label'	=> $row['label'] . '');
		$references[$idx]['id2'] = &$references[$idx]['id'];
		$references[$idx]['id'] += 1;
		$references[$idx++]['id2'] += 1;
	}

	$references[$idx++] = &$res;
	mysqli_free_result($res);
	@debug_zval_dump($references);

	if (!(mysqli_real_query($link, "SELECT id, label FROM test_mysqli_result_references_table_1 ORDER BY id ASC LIMIT 1")) ||
			!($res = mysqli_use_result($link)))
		printf("[004] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$tmp = array();
	while ($row = mysqli_fetch_assoc($res)) {
		$tmp[] = $row;
	}
	$tmp = unserialize(serialize($tmp));
	debug_zval_dump($tmp);
	mysqli_free_result($res);

	mysqli_close($link);
	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_result_references_table_1'; require_once("clean_table.inc");
?>