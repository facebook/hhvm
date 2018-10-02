<?php

<<__EntryPoint >>
function main_zlib_stream_filter() {
  // RFC1952
  $content = gzencode('rfc1952');
  $fh = fopen('php://memory', 'r+b');
  fwrite($fh, $content);
  rewind($fh);

  stream_filter_append($fh, 'zlib.inflate', STREAM_FILTER_READ, ['window' => 15 | 32]);

  var_dump(fread($fh, 1024));
  fclose($fh);

  // RFC1950
  $content = gzcompress('rfc1950');
  $fh = fopen('php://memory', 'r+b');
  fwrite($fh, $content);
  rewind($fh);

  stream_filter_append($fh, 'zlib.inflate', STREAM_FILTER_READ, ['window' => 15 | 32]);

  var_dump(fread($fh, 1024));
  fclose($fh);

  // RFC1951
  $content = gzdeflate('rfc1951');
  $fh = fopen('php://memory', 'r+b');
  fwrite($fh, $content);
  rewind($fh);

  stream_filter_append($fh, 'zlib.inflate', STREAM_FILTER_READ);

  var_dump(fread($fh, 1024));
  fclose($fh);

  // invalid window
  $content = gzencode('rfc1952');
  $fh = fopen('php://memory', 'r+b');
  fwrite($fh, $content);
  rewind($fh);
  stream_filter_append($fh, 'zlib.inflate', STREAM_FILTER_READ);
  var_dump(fread($fh, 1024));
  fclose($fh);
}
