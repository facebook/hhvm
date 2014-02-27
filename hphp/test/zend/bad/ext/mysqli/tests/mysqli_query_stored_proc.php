<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_query_stored_proc_table_1'; require_once('table.inc');

	if (!mysqli_query($link, 'DROP PROCEDURE IF EXISTS test_mysqli_query_stored_proc_procedure_1'))
		printf("[001] [%d] %s.\n", mysqli_errno($link), mysqli_error($link));

	if (mysqli_query($link, 'CREATE PROCEDURE test_mysqli_query_stored_proc_procedure_1() READS SQL DATA BEGIN SELECT id, label FROM test_mysqli_query_stored_proc_table_1 ORDER BY id ASC;
END;')) {
		/* stored proc which returns one result set */
		if (mysqli_multi_query($link, 'CALL test_mysqli_query_stored_proc_procedure_1()')) {
			do {
				if ($res = mysqli_use_result($link)) {
					// skip results, don't fetch all from server
					var_dump(mysqli_fetch_assoc($res));
					mysqli_free_result($res);
				}
			} while (mysqli_more_results($link) && mysqli_next_result($link));

		} else {
			printf("[003] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		}

		if (mysqli_multi_query($link, 'CALL test_mysqli_query_stored_proc_procedure_1()')) {
			do {
				if ($res = mysqli_store_result($link)) {
					// fetch all results from server, but skip on client side
					var_dump(mysqli_fetch_assoc($res));
					mysqli_free_result($res);
				}
			} while (mysqli_more_results($link) && mysqli_next_result($link));

		} else {
			printf("[004] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		}

		if (mysqli_multi_query($link, 'CALL test_mysqli_query_stored_proc_procedure_1()')) {
			do {
				if ($res = mysqli_store_result($link)) {
					// fetch all results from server, but skip on client side
					var_dump(mysqli_fetch_assoc($res));
					while (mysqli_fetch_assoc($res))
						;
					mysqli_free_result($res);
				}
			} while (mysqli_more_results($link) && mysqli_next_result($link));

		} else {
			printf("[005] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		}

	} else {
		printf("[002] Cannot create SP, [%d] %s.\n", mysqli_errno($link), mysqli_error($link));
	}

	if (!mysqli_query($link, 'DROP PROCEDURE IF EXISTS test_mysqli_query_stored_proc_procedure_1'))
		printf("[006] [%d] %s.\n", mysqli_errno($link), mysqli_error($link));

	if (mysqli_query($link, 'CREATE PROCEDURE test_mysqli_query_stored_proc_procedure_1() READS SQL DATA BEGIN SELECT id, label FROM test_mysqli_query_stored_proc_table_1 ORDER BY id ASC; SELECT id FROM test_mysqli_query_stored_proc_table_1 ORDER BY id ASC; END;')) {
		/* stored proc which returns two result sets */

		if (mysqli_multi_query($link, 'CALL test_mysqli_query_stored_proc_procedure_1()')) {
			do {
				if ($res = mysqli_store_result($link)) {
					// fetch all results from server, but skip on client side
					var_dump(mysqli_fetch_assoc($res));
					mysqli_free_result($res);
				}
			} while (mysqli_more_results($link) && mysqli_next_result($link));

		} else {
			printf("[008] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));
		}

	} else {
		printf("[007] Cannot create SP, [%d] %s.\n", mysqli_errno($link), mysqli_error($link));
	}

	if (!mysqli_query($link, 'DROP PROCEDURE IF EXISTS test_mysqli_query_stored_proc_procedure_1'))
		printf("[009] [%d] %s.\n", mysqli_errno($link), mysqli_error($link));

	if (mysqli_real_query($link, 'CREATE PROCEDURE test_mysqli_query_stored_proc_procedure_1(OUT ver_param VARCHAR(25)) BEGIN SELECT VERSION() INTO ver_param; END;')) {
		/* no result set, just output parameter */
		if (!mysqli_query($link, 'CALL test_mysqli_query_stored_proc_procedure_1(@version)'))
			printf("[011] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!mysqli_query($link, "SET @version = 'unknown'"))
			printf("[012] Cannot reset user variable, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!mysqli_query($link, 'CALL test_mysqli_query_stored_proc_procedure_1(@version)'))
			printf("[013] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!$res = mysqli_query($link, 'SELECT @version as _vers'))
			printf("[014] Cannot fetch user variable, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!$row = mysqli_fetch_assoc($res) ||
				$row['_vers'] == 'unknown')
			printf("[015] Results seem wrong, got %s, [%d] %s\n",
				$row['_vers'],
				mysqli_errno($link), mysqli_error($link));
		mysqli_free_result($res);

	} else {
		printf("[010] Cannot create SP, [%d] %s.\n", mysqli_errno($link), mysqli_error($link));
	}

	if (!mysqli_query($link, 'DROP PROCEDURE IF EXISTS test_mysqli_query_stored_proc_procedure_1'))
		printf("[016] [%d] %s.\n", mysqli_errno($link), mysqli_error($link));

	if (mysqli_real_query($link, 'CREATE PROCEDURE test_mysqli_query_stored_proc_procedure_1(IN ver_in VARCHAR(25), OUT ver_out VARCHAR(25)) BEGIN SELECT ver_in INTO ver_out; END;')) {
		/* no result set, one input, one output parameter */
		if (!mysqli_query($link, "CALL test_mysqli_query_stored_proc_procedure_1('myversion', @version)"))
			printf("[018] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!mysqli_query($link, "SET @version = 'unknown'"))
			printf("[019] Cannot reset user variable, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!mysqli_query($link, "CALL test_mysqli_query_stored_proc_procedure_1('myversion', @version)"))
			printf("[020] Cannot call SP, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!$res = mysqli_query($link, 'SELECT @version as _vers'))
			printf("[021] Cannot fetch user variable, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

		if (!$row = mysqli_fetch_assoc($res) ||
				$row['_vers'] == 'myversion')
			printf("[022] Results seem wrong, got %s, [%d] %s\n",
				$row['_vers'],
				mysqli_errno($link), mysqli_error($link));
		mysqli_free_result($res);

	} else {
		printf("[017] Cannot create SP, [%d] %s.\n", mysqli_errno($link), mysqli_error($link));
	}

	mysqli_close($link);
	print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

if (!mysqli_query($link, "DROP TABLE IF EXISTS test_mysqli_query_stored_proc_table_1"))
	printf("[c002] Cannot drop table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

@mysqli_query($link, "DROP PROCEDURE IS EXISTS test_mysqli_query_stored_proc_procedure_1");

mysqli_close($link);
?>