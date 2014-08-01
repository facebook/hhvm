<?php
	require_once("connect.inc");

	function sha_connect($offset, $host, $db, $port, $socket, $file) {

		$link = mysqli_init();
		if (!($link->options(MYSQLI_SERVER_PUBLIC_KEY, $file))) {
			printf("[%03d + 001] mysqli_options failed, [%d] %s\n", $offset, $link->errno, $link->error);
			return false;
		}

		if (!$link->real_connect($host, 'shatest', 'shatest', $db, $port, $socket)) {
			printf("[%03d + 002] [%d] %s\n", $offset, $link->connect_errno, $link->connect_error);
			return false;
		}

		if (!$res = $link->query("SELECT id FROM test WHERE id = 1"))
			printf("[%03d + 003] [%d] %s\n", $offset, $link->errno, $link->error);
			return false;

		if (!$row = mysqli_fetch_assoc($res)) {
			printf("[%03d + 004] [%d] %s\n", $offset, $link->errno, $link->error);
			return false;
		}

		if ($row['id'] != 1) {
			printf("[%03d + 005] Expecting 1 got %s/'%s'", $offset, gettype($row['id']), $row['id']);
			return false;
		}

		$res->close();
		$link->close();
		return true;
	}

	$file = sprintf("%s%s%s_%s", sys_get_temp_dir(), DIRECTORY_SEPARATOR, "test_sha256_" , @date("Ymd"));
	if (file_exists($file) && is_readable($file)) {

		/* valid key */
		sha_connect(100, $host, $db, $port, $socket, $file);

		/* invalid key */
		$file_wrong = sprintf("%s%s%s_%s", sys_get_temp_dir(), DIRECTORY_SEPARATOR, "test_sha256_wrong" , @date("Ymd"));

		$key = file_get_contents($file);
		$key = str_replace("A", "a", $key);
		$key = str_replace("M", "m", $key);
		@unlink($file_wrong);
		if (!($fp = fopen($file_wrong, "w"))) {
			printf("[002] Can't write public key file.");
		} else {
			fwrite($fp, $key);
			fclose($fp);
			sha_connect(200, $host, $db, $port, $socket, $file_wrong);
		}

		/* empty file */
		@unlink($file_wrong);
		if (!($fp = fopen($file_wrong, "w"))) {
			printf("[003] Can't write public key file.");
		} else {
			fwrite($fp, "");
			fclose($fp);
			sha_connect(300, $host, $db, $port, $socket, $file_wrong);
		}

		/* file does not exist */
		@unlink($file_wrong);
		sha_connect(400, $host, $db, $port, $socket, $file_wrong);

	} else {
		printf("[001] Cannot read public key file.");
	}

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_pam_sha256_public_key_option_invalid_table_1'; require_once("clean_table.inc");
	$link->query('DROP USER shatest');
	$link->query('DROP USER shatest@localhost');
	$file = sprintf("%s%s%s_%s", sys_get_temp_dir(), DIRECTORY_SEPARATOR, "test_sha256_" , @date("Ymd"));
	@unlink($file);
	$file_wrong = sprintf("%s%s%s_%s", sys_get_temp_dir(), DIRECTORY_SEPARATOR, "test_sha256_wrong" , @date("Ymd"));
	@unlink($file_wrong);
?>