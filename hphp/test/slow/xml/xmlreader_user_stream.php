<?php


class StringWrapper {
  private $buf = '<xml><blob/></xml>';

  public function stream_open($path, $mode, $options, &$opened_path) {
    return true;
  }

  public function stream_eof() {
    return $this->buf === '';
  }

  public function stream_read($size) {
    $bytes = min($size, strlen($this->buf));
    $ret = substr($this->buf, 0, $bytes);
    $this->buf = substr($this->buf, $bytes);
    return $ret;
  }
}

function main() {
  var_dump(stream_wrapper_register('strstream', 'StringWrapper'));
  $r = new XMLReader();
  $r->open("strstream://");
  var_dump($r->read());
  var_dump($r->readOuterXML());
  var_dump($r->readInnerXML());
  var_dump($r->read());
  var_dump($r->readOuterXML());
  var_dump($r->readInnerXML());
}
main();
