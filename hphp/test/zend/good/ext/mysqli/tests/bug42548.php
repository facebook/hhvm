<?php
require_once('connect.inc');

$mysqli = mysqli_init();
$mysqli->real_connect($host, $user, $passwd, $db, $port, $socket);
if (mysqli_connect_errno()) {
	printf("Connect failed: %s\n", mysqli_connect_error());
	exit();
}

$mysqli->query("DROP PROCEDURE IF EXISTS test_bug42548_procedure_1") or die($mysqli->error);
$mysqli->query("CREATE PROCEDURE test_bug42548_procedure_1() BEGIN SELECT 23; SELECT 42; END") or die($mysqli->error);

if ($mysqli->multi_query("CALL test_bug42548_procedure_1();"))
{
	do
	{
		if ($objResult = $mysqli->store_result()) {
			while ($row = $objResult->fetch_assoc()) {
				print_r($row);
			}
			$objResult->close();
			if ($mysqli->more_results()) {
				print "----- next result -----------\n";
			}
		} else {
			print "no results found\n";
		}
	} while ($mysqli->more_results() && $mysqli->next_result());
} else {
	print $mysqli->error;
}

$mysqli->query("DROP PROCEDURE test_bug42548_procedure_1") or die($mysqli->error);
$mysqli->close();
print "done!";
?>
<?php
require_once("connect.inc");
if (!$link = my_mysqli_connect($host, $user, $passwd, $db, $port, $socket))
   printf("[c001] [%d] %s\n", mysqli_connect_errno(), mysqli_connect_error());

mysqli_query($link, "DROP PROCEDURE IF EXISTS test_bug42548_procedure_1");

mysqli_close($link);
?>