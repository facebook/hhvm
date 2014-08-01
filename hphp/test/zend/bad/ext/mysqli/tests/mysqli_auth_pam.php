<?php
	require_once('connect.inc');
	$test_table_name = 'test_mysqli_auth_pam_table_1'; require_once('table.inc');

	if (!$link = my_mysqli_connect($host, 'pamtest', 'pamtest', $db, $port, $socket)) {
		printf("[001] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);
	} else {

	  if (!$res = $link->query("SELECT id FROM test WHERE id = 1"))
		  printf("[002] [%d] %s\n", $link->errno, $link->error);

	  if (!$row = mysqli_fetch_assoc($res)) {
		  printf("[003] [%d] %s\n", $link->errno, $link->error);
	  }

	  if ($row['id'] != 1) {
		  printf("[004] Expecting 1 got %s/'%s'", gettype($row['id']), $row['id']);
	  }

	  $res->close();
	  $link->close();
	}

	print "done!";
?>
<?php
	$test_table_name = 'test_mysqli_auth_pam_table_1'; require_once("clean_table.inc");
	mysqli_query($link, 'DROP USER pamtest');
	mysqli_query($link, 'DROP USER pamtest@localhost');
?>