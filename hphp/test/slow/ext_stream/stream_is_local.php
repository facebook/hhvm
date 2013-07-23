<?php

class MyWrapper {
  public function __construct() {
  }
  public function stream_open($fn, $mode, $opt, &$opened_path) {
    return true;
  }
}
stream_wrapper_register('wrap', 'MyWrapper');

var_dump(stream_is_local(4));
var_dump(stream_is_local(""));
var_dump(stream_is_local("\n"));
var_dump(stream_is_local(null));
var_dump(stream_is_local("http://example.com"));

foreach (array(
  __FILE__,
  "wrap://test",
) as $file) {
  var_dump(stream_is_local($file));
  var_dump(stream_is_local(fopen($file, 'r')));
}
