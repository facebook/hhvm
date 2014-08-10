<?php
	require_once("connect.inc");

	$file = sprintf("%s%s%s_%s", sys_get_temp_dir(), DIRECTORY_SEPARATOR, "test_sha256_" , @date("Ymd"));
	if (file_exists($file) && is_readable($file)) {

		$link = mysqli_init();
		if (!($link->options(MYSQLI_SERVER_PUBLIC_KEY, $file))) {
			printf("[001] mysqli_options failed, [%d] %s\n", $link->errno, $link->error);
		}

		if (!$link->real_connect($host, 'shatest', 'shatest', $db, $port, $socket)) {
			printf("[002] [%d] %s\n", $link->connect_errno, $link->connect_error);
		}

		if (!$res = $link->query("SELECT id FROM test WHERE id = 1"))
			printf("[003] [%d] %s\n", $link->errno, $link->error);

		if (!$row = mysqli_fetch_assoc($res)) {
			printf("[004] [%d] %s\n", $link->errno, $link->error);
		}

		if ($row['id'] != 1) {
			printf("[005] Expecting 1 got %s/'%s'", gettype($row['id']), $row['id']);
		}

		$res->close();
		$link->close();
	}

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_pam_sha256_public_key_option_table_1'; require_once("clean_table.inc");
	$link->query('DROP USER shatest');
	$link->query('DROP USER shatest@localhost');
	$file = sprintf("%s%s%s_%s", sys_get_temp_dir(), DIRECTORY_SEPARATOR, "test_sha256_" , @date("Ymd"));
	@unlink($file);
?>