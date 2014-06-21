<?php

$key = "TEST_KEY_COMPRESSION";
$tmp_object = new stdClass;
$tmp_object->str_attr = str_repeat("0", 100);
$tmp_object->int_attr = 123;

$serialized = serialize($tmp_object);
$compressed = zlib_encode($serialized, ZLIB_ENCODING_DEFLATE);

$socket = stream_socket_client('localhost:11211');
fwrite($socket, "set ". $key . " 3 0 " . strlen($compressed) . "\r\n" .
       $compressed . "\r\n" );
$response = trim(stream_get_contents($socket));
if ($response != 'STORED') {
  echo "Memcache write error: $response\n";
}
fclose($socket);

$memcache = new Memcache;
$memcache->addServer('localhost', 11211);
$r = $memcache->get($key);
var_dump($r);
$memcache->delete($key);
