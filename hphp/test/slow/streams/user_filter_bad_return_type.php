<?php

function filter_impl($in, $out) {
  while ($bucket = stream_bucket_make_writeable($in)) {
    $bucket->data = strtoupper($bucket->data).$bucket->data;
    $consumed += $bucket->dataLen;
    $bucket->dataLen *= 2;
    stream_bucket_append($out, $bucket);
  }
}

class FatalFilter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    filter_impl($in, $out);
    return PSFS_ERR_FATAL;
  }
}

class StrPassFilter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    filter_impl($in, $out);
    return (string) PSFS_PASS_ON;
  }
}

class BoolPassFilter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    filter_impl($in, $out);
    return (bool) PSFS_PASS_ON;
  }
}

class NullFilter extends php_user_filter {
  function filter($in, $out, &$consumed, $closing) {
    filter_impl($in, $out);
    return null;
  }
}

function main() {
  $filters = array(
    'FatalFilter',
    'StrPassFilter',
    'BoolPassFilter',
    'NullFilter'
  );
  foreach ($filters as $filter) {
    printf("---%s---\n", $filter);
    stream_filter_register($filter, $filter);
    $f = fopen('php://memory', 'r+');
    stream_filter_append($f, $filter, STREAM_FILTER_READ);
    fwrite($f, 'foo');
    rewind($f);
    var_dump(fread($f, 1024));
  }
}

main();
