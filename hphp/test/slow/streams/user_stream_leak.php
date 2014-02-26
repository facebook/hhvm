<?php

class test {
  function stream_open() { echo "open\n"; return true; }
  function stream_write($d) { echo "write $d\n"; return strlen($d); }
  function stream_flush() { echo "flush\n"; return true; }
  function stream_close() { echo "close\n"; }
}
stream_register_wrapper("test", "test", STREAM_IS_URL);
var_dump(file_put_contents("test://hello", "w"));
