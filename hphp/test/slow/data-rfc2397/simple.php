<?php

// originally based on ext/standard/tests/file/stream_rfc2397_001.phpt

$data = 'data://,hello world';

var_dump(file_get_contents($data));

$file = fopen($data, 'r');
unset($data);

var_dump(stream_get_contents($file));

$streams = array(
  'data://text/plain;base64,SSBsb3ZlIFBIUAo=',
  'data:text/plain;base64,SSBsb3ZlIFBIUAo=',
  "data:,A%20brief%20note",
  "data:text/plain;charset=iso-8859-7,%be%fg%be",
);

foreach ($streams as $stream) {
  var_dump(file_get_contents($stream));
}
