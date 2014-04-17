<?php

class Foo extends php_user_filter {
  public function filter($in, $out, &$consumed, $closing) {
    while ($bucket = stream_bucket_make_writeable($in)) {
      stream_bucket_append(
        $out,
        stream_bucket_new($this->stream, strtoupper($bucket->data))
      );
    }
    return PSFS_FEED_ME;
  }
}

class Bar extends php_user_filter {
  public function filter($in, $out, &$consumed, $closing) {
    while ($bucket = stream_bucket_make_writeable($in)) {
      stream_bucket_append(
        $out,
        stream_bucket_new($this->stream, strtoupper($bucket->data))
      );
    }
    return PSFS_PASS_ON;
  }
}

function main() {
  stream_filter_register('Foo', 'Foo');
  stream_filter_register('Bar', 'Bar');

  $f = fopen('php://memory', 'r+');
  $filter = stream_filter_append($f, 'Foo', STREAM_FILTER_WRITE);
  fwrite($f, 'herp');
  stream_filter_remove($filter);
  rewind($f);
  var_dump(fread($f, 1024));

  $f = fopen('php://memory', 'r+');
  $filter = stream_filter_append($f, 'Bar', STREAM_FILTER_WRITE);
  fwrite($f, 'herp');
  stream_filter_remove($filter);
  rewind($f);
  var_dump(fread($f, 1024));
}

main();
