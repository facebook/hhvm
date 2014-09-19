<?php
require 'server.inc';

function do_test($version, $connection) {
    $options = [
        'http' => [
            'protocol_version' => $version,
        ],
    ];

    if ($connection) {
        $options['http']['header'] = "Connection: $connection";
    }

    $ctx = stream_context_create($options);

    $responses = ["data://text/plain,HTTP/$version 204 No Content\r\n\r\n"];
    $pid = http_server('tcp://127.0.0.1:12342', $responses, $output);

    $fd = fopen('http://127.0.0.1:12342/', 'rb', false, $ctx);
    fseek($output, 0, SEEK_SET);
    echo stream_get_contents($output);

    http_server_kill($pid);
}

echo "HTTP/1.0, default behaviour:\n";
do_test('1.0', null);

echo "HTTP/1.0, connection: close:\n";
do_test('1.0', 'close');

echo "HTTP/1.0, connection: keep-alive:\n";
do_test('1.0', 'keep-alive');

echo "HTTP/1.1, default behaviour:\n";
do_test('1.1', null);

echo "HTTP/1.1, connection: close:\n";
do_test('1.1', 'close');

echo "HTTP/1.1, connection: keep-alive:\n";
do_test('1.1', 'keep-alive');
?>
