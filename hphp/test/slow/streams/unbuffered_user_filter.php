<?php

class TestFilter extends php_user_filter {
  public function filter(
    $in,
    $out,
    &$consumed,
    $params
  ) {
    $consumed = 0;
    while ($bucket = stream_bucket_make_writeable($in)) {
      $bucket->data = strtoupper($bucket->data).$bucket->data;
      $consumed += $bucket->dataLen;
      $bucket->dataLen *= 2;
      stream_bucket_append($out, $bucket);
    }
    return PSFS_PASS_ON;
  }
}

var_dump(stream_filter_register('testfilter', 'TestFilter'));

function testWrite() {
  printf("---%s---\n", __FUNCTION__);
  $f = fopen('php://memory', 'r+');
  stream_filter_append($f, 'testfilter', STREAM_FILTER_WRITE);
  fwrite($f, 'abcd');
  rewind($f);
  var_dump(fread($f, 1024));
}

function testSimpleRead() {
  printf("---%s---\n", __FUNCTION__);
  $f = fopen('php://memory', 'r+');
  stream_filter_append($f, 'testfilter', STREAM_FILTER_READ);
  fwrite($f, 'abcd');
  rewind($f);
  var_dump(fread($f, 1024));
}

function testStagedRead() {
  printf("---%s---\n", __FUNCTION__);
  $f = fopen('php://memory', 'r+');
  stream_filter_append($f, 'testfilter', STREAM_FILTER_READ);
  fwrite($f, 'abcd');
  rewind($f);
  var_dump(fread($f, 4));
  var_dump(fread($f, 4));
}

function main() {
  testWrite();
  testSimpleRead();
  testStagedRead();
}

main();
