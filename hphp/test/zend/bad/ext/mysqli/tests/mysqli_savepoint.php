<?php
	require_once("connect.inc");
	 /* {{{ proto bool mysqli_savepoint(object link, string name) */
	$tmp    = NULL;
	$link   = NULL;
	if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
		printf("[003] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	$name = array();
	if (false !== ($tmp = mysqli_savepoint($link, '')))
		printf("[006] Expecting false, got %s/%s\n", gettype($tmp), $tmp);

	if (!mysqli_query($link, 'DROP TABLE IF EXISTS test'))
		printf("[007] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, 'CREATE TABLE test(id INT) ENGINE = InnoDB'))
		printf("[008] Cannot create test table, [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (true !== ($tmp = mysqli_autocommit($link, false)))
		printf("[009] Cannot turn off autocommit, expecting true, got %s/%s\n", gettype($tmp), $tmp);

	/* overrule autocommit */
	if (true !== ($tmp = mysqli_savepoint($link, 'my')))
		printf("[010] Got %s - [%d] %s\n", var_dump($tmp, true), mysqli_errno($link), mysqli_error($link));

	if (!mysqli_query($link, 'INSERT INTO test(id) VALUES (1)'))
		printf("[011] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	$tmp = mysqli_rollback($link);
	if ($tmp !== true)
		printf("[012] Expecting boolean/true, got %s/%s\n", gettype($tmp), $tmp);

	print "done!";
?>
<?php error_reporting(0); ?>
<?php
	$test_table_name = 'test_mysqli_savepoint_table_1'; require_once("clean_table.inc");
?>