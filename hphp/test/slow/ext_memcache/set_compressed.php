<?php

$key = "TEST_KEY_COMPRESSION_SET";
$tmp_object = new stdClass;
$tmp_object->str_attr = "אבגabcéàç";
$tmp_object->str2 = str_repeat("0", 100);
$tmp_object->int_attr = 123;

$serialized = serialize($tmp_object);
$compressed = zlib_encode($serialized, ZLIB_ENCODING_DEFLATE);

$memcache = new Memcache;
$memcache->addServer('localhost', 11211);
$memcache->set($key, $tmp_object, MEMCACHE_COMPRESSED);

$socket = stream_socket_client('localhost:11211');
fwrite($socket, "get " . $key . "\r\n");
$line1 = fgets( $socket );
$line2 = fgets( $socket );
$line3 = fgets( $socket );
fclose($socket);

$line1_parts = explode(' ', substr($line1, 0, -2));
$line2 = substr($line2, 0, -2);
var_dump($line1_parts);
var_dump($line2 === $compressed);

$memcache->delete($key);
