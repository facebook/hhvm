<?php

class FifoStream {
  private $data;
  function stream_open($filename, $mode, $options, &$opened_path) {
    echo "Open\n";
    var_dump($filename, $mode, $options & USE_INCLUDE_PATH);
    return true;
  }
  function stream_write($buf) {
 $this->data .= $buf;
 }
  function stream_read($count) {
    $chunk = substr($this->data, 0, $count);
    $this->data = substr($this->data, $count);
    return $chunk;
  }
  function stream_eof() {
 return strlen($this->data) == 0;
 }
  function stream_flush() {
 $this->data = '';
 }
  function stream_close() {
 echo "Close\n";
 }
}
var_dump(stream_wrapper_register('fifo', 'FifoStream'));
$fp = fopen('fifo://testing', 'w+');
var_dump(fwrite($fp, "Data one...\n"));
fflush($fp);
var_dump(fwrite($fp, "Data two...\n"));
var_dump(fwrite($fp, "Data three...\n"));
while(!feof($fp)) {
  var_dump(fgets($fp));
}
fclose($fp);
