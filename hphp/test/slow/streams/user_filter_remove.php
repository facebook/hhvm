<?php

class ClosingFilter extends php_user_filter {
  public $buffer = '';

  public function filter($in, $out, &$consumed, $closing) {
    var_dump('filtering');
    while ($bucket = stream_bucket_make_writeable($in)) {
      $this->buffer .= $bucket->data;
      $consumed += $bucket->datalen;
    }
    if ($closing) {
      var_dump('closing');
      $bucket = stream_bucket_new(
        $this->stream,
        '<Closed: '.$this->buffer.'>'
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
  $filter = stream_filter_append($f, 'ClosingFilter', STREAM_FILTER_WRITE);
  fwrite($f, 'foo bar');
  fwrite($f, 'herp derp');
  var_dump('written, removing');
  stream_filter_remove($filter);
  var_dump('removed');
  fwrite($f, 'post filter');
  fclose($f);

  var_dump(file_get_contents($fname));
  unlink($fname);
}

main();
