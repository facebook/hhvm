<?php

class MyStream {
  public $context;

  public function stream_open() {
    var_dump($this->context);
    print_r(stream_context_get_options($this->context));
    return true;
  }
  public function stream_stat() { return false; }
  public function stream_read($maxLen) { return ''; }
  public function stream_eof() {  return true; }
}

stream_wrapper_register('my', 'MyStream');
$fp = fopen('my://stream', 'r', false, stream_context_create(['my' => ['foo' => 'bar']]));
