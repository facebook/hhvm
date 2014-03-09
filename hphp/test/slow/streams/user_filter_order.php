<?php

class Md5Filter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    while ($ib = stream_bucket_make_writeable($in)) {
      $ob = stream_bucket_new($this->stream, md5($ib->data));
      stream_bucket_append($out, $ob);
    }
    return PSFS_PASS_ON;
  }
}

class UpperCaseFilter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    while ($ib = stream_bucket_make_writeable($in)) {
      $ob = stream_bucket_new($this->stream, strtoupper($ib->data));
      stream_bucket_append($out, $ob);
    }
    return PSFS_PASS_ON;
  }
}

function doTest($func) {
  $f = fopen('php://memory', 'r+');
  fwrite($f, 'herp derp');
  rewind($f);
  $func($f, 'test.md5', STREAM_FILTER_READ);
  $func($f, 'test.ucase', STREAM_FILTER_READ);
  var_dump(fread($f, 1024));
}

function main() {
  stream_filter_register('test.md5', 'Md5Filter');
  stream_filter_register('test.ucase', 'UpperCaseFilter');
  doTest('stream_filter_append');
  doTest('stream_filter_prepend');
}

main();
