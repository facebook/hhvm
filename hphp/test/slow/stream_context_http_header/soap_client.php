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
$soapResponse = '<?xml version="1.0" encoding="UTF-8"?>'
    . '<SOAP-ENV:Envelope '
    . 'xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" '
    . 'xmlns:SOAP-ENC="http://schemas.xmlsoap.org/soap/encoding/" '
    . 'xmlns:ns1="http://testing.dev" '
    . 'xmlns:xsd="http://www.w3.org/2001/XMLSchema" '
    . 'xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" '
    . 'SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">'
    . '<SOAP-ENV:Body>'
    . '<ns1:getRequestHeadersResponse/>'
    . '</SOAP-ENV:Body>'
    . '</SOAP-ENV:Envelope>';
$contentLength = strlen($soapResponse);
$response = "HTTP/1.0 200 OK\r\n"
    . "Content-Type: text/xml; charset=utf-8\r\n"
    . "Content-Length: $contentLength\r\n"
    . "\r\n"
    . "$soapResponse\r\n";
$responses = array_fill(0, count($headers), $response);

$pid = httpServer("tcp://127.0.0.1:$port", $responses, $output);

foreach ($headers as $header) {
    $context = stream_context_create(['http' => ['header' => $header]]);

    $client = new SoapClient(null, [
        'location' => $url,
        'stream_context' => $context,
        'style' => SOAP_RPC, # PHP will default to SOAP_RPC, HHVM defaults to an empty SOAP body
        'uri' => 'http://testing.dev',
    ]);

    try {
        $client->__soapCall('getRequestHeaders', array());
    } catch (SoapFault $e) {
        echo get_class($e) . ': [' . $e->faultcode . '] ' . $e->getMessage() . "\n";
        continue;
    }
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
