<?php
echo "Test\n";
$server = stream_socket_server("unix://\x00/MyBindName");
$client = stream_socket_client("unix://\x00/MyBindName");
if ($client) {
	echo "ok\n";
}
?>
===DONE===
