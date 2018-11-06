<?php
require 'random_port.inc';
require 'http_server.inc';

$port = random_free_port();
$url = "http://127.0.0.1:$port/test";
$headers = [
    'X-TEST-STRING-1: value1',
    "X-TEST-STRING-2: value2\r\nX-TEST-STRING-3: value3",
    ['X-TEST-ELEMENT-1: element1', 'X-TEST-ELEMENT-2: element2']
];
$response = "HTTP/1.0 200 OK\r\n";
$responses = array_fill(0, count($headers), $response);

$pid = httpServer("tcp://127.0.0.1:$port", $responses, $output);

foreach ($headers as $header) {
    $context = stream_context_create(['http' => ['header' => $header]]);
    file_get_contents($url, null, $context);
}

httpServerKill($pid);

fseek($output, 0, SEEK_SET);
$lines = explode("\r\n", trim(stream_get_contents($output)));
sort($lines);
foreach ($lines as $line) {
    if (substr($line, 0, 7) != 'X-TEST-') {
        continue;
    }

    var_dump($line);
}
