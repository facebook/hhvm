<?php
ini_set('allow_url_include', 1);

ini_set('allow_url_fopen', 1);

require 'server.inc';

function do_test() {

	$responses = array(
		"data://text/plain,HTTP/1.0 404 Not Found\r\n\r\n",
		"data://text/plain,HTTP/1.0 404 Not Found\r\n\r\n",
		"data://text/plain,HTTP/1.0 404 Not Found\r\n\r\n"
	);

	$pid = http_server("tcp://127.0.0.1:12342", $responses, $output);

	$a = $b = null;

	$i = 3;
	while ($i--) {
		$context = stream_context_create(array('http'=>array('timeout'=>1)));
		file_get_contents('http://127.0.0.1:12342/', 0, $context);
		unset($context);
		
		$b = $a;
		$a = memory_get_usage();
	}

	http_server_kill($pid);

	echo "leak? penultimate iteration: $b, last one: $a\n";
	var_dump($a == $b);
}

do_test();
