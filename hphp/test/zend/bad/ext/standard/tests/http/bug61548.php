<?php
ini_set('allow_url_fopen', 1);

require 'server.inc';

function do_test($header) {
    $options = [
        'http' => [
			'method' => 'POST',
			'header' => $header,
            'follow_location' => true,
        ],
    ];

    $ctx = stream_context_create($options);

    $responses = [
		"data://text/plain,HTTP/1.1 201\r\nLocation: /foo\r\n\r\n",
		"data://text/plain,HTTP/1.1 200\r\nConnection: close\r\n\r\n",
	];
    $pid = http_server('tcp://127.0.0.1:12342', $responses, $output);

    $fd = fopen('http://127.0.0.1:12342/', 'rb', false, $ctx);
    fseek($output, 0, SEEK_SET);
    echo stream_get_contents($output);

    http_server_kill($pid);
}

do_test("First:1\nSecond:2\nContent-type: text/plain");
do_test("First:1\nSecond:2\nContent-type: text/plain\n");
do_test("First:1\nSecond:2\nContent-type: text/plain\nThird:");
do_test("First:1\nContent-type:text/plain\nSecond:2");
do_test("First:1\nContent-type:text/plain\nSecond:2\n");
do_test("First:1\nContent-type:text/plain\nSecond:2\nThird:");

?>
Done