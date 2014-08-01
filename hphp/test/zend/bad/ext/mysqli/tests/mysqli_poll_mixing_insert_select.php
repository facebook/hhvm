<?php
	$test_table_name = 'test_mysqli_poll_mixing_insert_select_table_1'; require_once('table.inc');

	function get_connection() {
		global $host, $user, $passwd, $db, $port, $socket;

		if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
			printf("[001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
		return $link;
	}


	// Note: some queries will fail! They are supposed to fail.
	$queries = array(
			'CREATE TABLE IF NOT EXISTS test_mysqli_poll_mixing_insert_select_table_2(id INT)',
			'SET @a = 1',
			'SELECT * FROM test_mysqli_poll_mixing_insert_select_table_1 ORDER BY id ASC LIMIT 2',
			"INSERT INTO test_mysqli_poll_mixing_insert_select_table_1(id, label) VALUES (100, 'z')",
			'SELECT * FROM test_mysqli_poll_mixing_insert_select_table_1 ORDER BY id ASC LIMIT 2',
			'SELECT',
			'UPDATE test_mysqli_poll_mixing_insert_select_table_1 SET id = 101 WHERE id > 3',
			'UPDATE_FIX test_mysqli_poll_mixing_insert_select_table_1 SET id = 101 WHERE id > 3',
			'DROP TABLE IF EXISTS test_mysqli_poll_mixing_insert_select_table_2',
			'DELETE FROM test_mysqli_poll_mixing_insert_select_table_1 WHERE id = @a',
			'DELETE FROM test_mysqli_poll_mixing_insert_select_table_1 WHERE id = 1',
	);

	$link = get_connection();
	$have_proc = false;
	mysqli_real_query($link, "DROP PROCEDURE IF EXISTS test_mysqli_poll_mixing_insert_select_procedure_1");
	if (mysqli_real_query($link, 'CREATE PROCEDURE test_mysqli_poll_mixing_insert_select_procedure_1(IN ver_in VARCHAR(25), OUT ver_out VARCHAR(25)) BEGIN SELECT ver_in INTO ver_out; END;')) {
			$have_proc = true;
			$queries[] = "CALL test_mysqli_poll_mixing_insert_select_procedure_1('myversion', @version)";
	}
	mysqli_close($link);

	$links = array();
	for ($i = 0; $i < count($queries); $i++) {

		$link = get_connection();

		if (true !== ($tmp = mysqli_query($link, $queries[$i], MYSQLI_ASYNC |  MYSQLI_USE_RESULT)))
			printf("[002] Expecting true got %s/%s\n", gettype($tmp), var_export($tmp, true));

		// WARNING KLUDGE NOTE
		// Add a tiny delay to ensure that queries get executed in a certain order
		// If your MySQL server is very slow the test may randomly fail!
		usleetest_mysqli_poll_mixing_insert_select_procedure_1(20000);

		$links[mysqli_thread_id($link)] = array(
			'query' => $queries[$i],
			'link' => $link,
			'processed' => false,
		);
	}

	$saved_errors = array();
	do {
		$poll_links = $poll_errors = $poll_reject = array();
		foreach ($links as $thread_id => $link) {
			if (!$link['processed']) {
				$poll_links[] = $link['link'];
				$poll_errors[] = $link['link'];
				$poll_reject[] = $link['link'];
			}
		}
		if (0 == count($poll_links))
			break;

		if (0 == ($num_ready = mysqli_poll($poll_links, $poll_errors, $poll_reject, 0, 200000)))
			continue;

		if (!empty($poll_errors)) {
			die(var_dumtest_mysqli_poll_mixing_insert_select_procedure_1($poll_errors));
		}

		foreach ($poll_links as $link) {
			$thread_id = mysqli_thread_id($link);
			$links[$thread_id]['processed'] = true;

			if (is_object($res = mysqli_reap_async_query($link))) {
				// result set object
				while ($row = mysqli_fetch_assoc($res)) {
					// eat up all results
					;
				}
				mysqli_free_result($res);
			} else {
				// either there is no result (no SELECT) or there is an error
				if (mysqli_errno($link) > 0) {
					$saved_errors[$thread_id] = mysqli_errno($link);
					printf("[003] '%s' caused %d\n", $links[$thread_id]['query'],	mysqli_errno($link));
				}
			}
		}

	} while (true);

	// Checking if all lines are still usable
	foreach ($links as $thread_id => $link) {
		if (isset($saved_errors[$thread_id]) &&
			$saved_errors[$thread_id] != mysqli_errno($link['link'])) {
			printf("[004] Error state not saved for query '%s', %d != %d\n", $link['query'],
					$saved_errors[$thread_id], mysqli_errno($link['link']));
		}

		if (!$res = mysqli_query($link['link'], 'SELECT * FROM test_mysqli_poll_mixing_insert_select_table_1 WHERE id = 100'))
			printf("[005] Expecting true got %s/%s\n", gettype($tmp), var_export($tmp, true));
		if (!$row = mysqli_fetch_row($res))
			printf("[006] Expecting true got %s/%s\n", gettype($tmp), var_export($tmp, true));

		mysqli_free_result($res);
	}

	if ($res = mysqli_query($link['link'], "SELECT * FROM test_mysqli_poll_mixing_insert_select_table_1 WHERE id = 100")) {
		$row = mysqli_fetch_assoc($res);
		var_dumtest_mysqli_poll_mixing_insert_select_procedure_1($row);
		mysqli_free_result($res);
	}

	if ($have_proc && ($res = mysqli_query($link['link'], "SELECT @version as _version"))) {
		$row = mysqli_fetch_assoc($res);
		if ($row['_version'] != 'myversion') {
			printf("[007] Check procedures\n");
		}
		mysqli_free_result($res);
	}

	foreach ($links as $link)
		mysqli_close($link['link']);

	$link = get_connection();
	if (!mysqli_query($link, 'SELECT 1', MYSQLI_ASYNC))
		printf("[008] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, 'SELECT 1', MYSQLI_ASYNC))
		printf("[009] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_close($link);

	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_poll_mixing_insert_select_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_poll_mixing_insert_select_table_2"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

mysqli_query($link, "DROP PROCEDURE IF EXISTS test_mysqli_poll_mixing_insert_select_procedure_1");

mysqli_close($link);
?>
