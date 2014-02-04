<?php
	include ("connect.inc");

	$link = mysqli_init();
	if (!my_mysqli_real_connect($link, $host, $user, $passwd, $db, $port, $socket)) {
		printf("[002] Connect failed, [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());
	}

	$create = "CREATE TEMPORARY TABLE IF NOT EXISTS test_bug54221_table_1(a int)";

	$query = "$create;$create;$create;";
	if ($link->multi_query($query)) {
		do {
			$sth = $link->store_result();

			if ($link->warning_count) {
				$warnings = $link->get_warnings();
				if ($warnings) {
					do {
						echo "Warning: ".$warnings->errno.": ".$warnings->message."\n"; 
					} while ($warnings->next());
				}
			}
		} while ($link->more_results() && $link->next_result());
	}

	mysqli_close($link);

	print "done!";
?>