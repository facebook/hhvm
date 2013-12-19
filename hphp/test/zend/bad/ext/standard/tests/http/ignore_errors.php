<?php
ini_set('allow_url_fopen', 1);

require 'server.inc';

function do_test($context_options) {

	$context = stream_context_create(array('http' => $context_options));

	$responses = array(
		"data://text/plain,HTTP/1.0 200 Ok\r\nX-Foo: bar\r\n\r\n1",
		"data://text/plain,HTTP/1.0 404 Not found\r\nX-bar: baz\r\n\r\n2",
	);

	$pid = http_server("tcp://127.0.0.1:12342", $responses, $output);

	foreach($responses as $r) {

		$fd = fopen('http://127.0.0.1:12342/foo/bar', 'rb', false, $context);
		var_dump($fd);

		if ($fd) {
			$meta_data = stream_get_meta_data($fd);
			var_dump($meta_data['wrapper_data']);
	
			var_dump(stream_get_contents($fd));
		}

		fseek($output, 0, SEEK_SET);
		var_dump(stream_get_contents($output));
		fseek($output, 0, SEEK_SET);
	}

	http_server_kill($pid);
}

echo "-- Test: requests without ignore_errors --\n";

do_test(array());

echo "-- Test: requests with ignore_errors --\n";

do_test(array('ignore_errors' => true));

echo "-- Test: requests with ignore_errors (2) --\n";

do_test(array('ignore_errors' => 1));

?>