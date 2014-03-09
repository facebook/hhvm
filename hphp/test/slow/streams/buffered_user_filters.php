<?php

class ClosingFilter extends php_user_filter {
  public $buffer = '';

  public function filter($in, $out, &$consumed, $closing) {
    while ($bucket = stream_bucket_make_writeable($in)) {
      $this->buffer .= $bucket->data;
      $consumed += $bucket->dataLen;
    }
    if ($closing) {
      $bucket = stream_bucket_new(
        $this->stream,
        'Closed: '. $this->buffer
      );
      stream_bucket_append($out, $bucket);
    }
    return PSFS_PASS_ON;
  }
}

function main() {
  $fname = tempnam('/tmp', 'foo');

  stream_filter_register('ClosingFilter', 'ClosingFilter');
  $f = fopen($fname, 'r+');
  stream_filter_append($f, 'ClosingFilter', STREAM_FILTER_WRITE);
  fwrite($f, 'foo bar');
  fwrite($f, 'herp derp');
  fclose($f);

  var_dump(file_get_contents($fname));
  unlink($fname);
}

main();
