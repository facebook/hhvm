<?php
ini_set('from', teste@teste.pt);

ini_set('allow_url_fopen', 1);

require 'server.inc';

function do_test() {

	$responses = array(
		"data://text/plain,HTTP/1.0 200 OK\r\n\r\n",
	);

	$pid = http_server("tcp://127.0.0.1:12342", $responses, $output);

	foreach($responses as $r) {

		$fd = fopen('http://127.0.0.1:12342/', 'rb', false);

		fseek($output, 0, SEEK_SET);
		var_dump(stream_get_contents($output));
		fseek($output, 0, SEEK_SET);
	}

	http_server_kill($pid);

}

echo "-- Test: leave default --\n";

do_test();

echo "-- Test: after ini_set --\n";

ini_set('from', 'junk@junk.com');

do_test();

?>