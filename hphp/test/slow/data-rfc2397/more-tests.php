<?php

// originally based on ext/standard/tests/file/stream_rfc2397_005.phpt

$streams = array(
  'data://,;test',
  'data://text/plain,test',
  'data://text/plain;charset=US-ASCII,test',
  'data://;charset=UTF-8,Hello',
  'data://text/plain;charset=UTF-8,Hello',
  'data://,a,b',
);

foreach ($streams as $stream) {
  var_dump(@file_get_contents($stream));
}
