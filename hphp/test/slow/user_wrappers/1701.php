<?php

class MemoryStream {
  private $data;
  private $ofs = 0;
  function stream_open($filename, $mode, $options, &$opened_path) {
    if (strncmp($filename, "mem://", 6)) {
 return false;
 }
    $this->data = substr($filename, 6);
    return true;
  }
  function stream_read($count) {
    $ret = substr($this->data, $this->ofs, $count);
    $this->ofs += $count;
    if ($this->ofs > strlen($this->data))      $this->ofs = strlen($this->data);
    return $ret;
  }
  function stream_seek($ofs, $whence) {
    if ($whence == SEEK_CUR) $this->ofs += $ofs;
    if ($whence == SEEK_SET) $this->ofs  = $ofs;
    if ($whence == SEEK_END) $this->ofs  = strlen($this->data) + $ofs;
    if ($this->ofs < 0) $this->ofs = 0;
    if ($this->ofs > strlen($this->data))      $this->ofs = strlen($this->data);
    return true;
  }
  function stream_tell() {
 return $this->ofs;
 }
}
stream_wrapper_register('mem', 'MemoryStream');
$fp = fopen('mem://abcdefghijklmnopqrstuvwxyz', 'r');
var_dump(fgetc($fp), fgetc($fp));
fseek($fp, 11, SEEK_CUR);
var_dump(fgetc($fp), fgetc($fp));
fseek($fp, 0, SEEK_END);
var_dump(ftell($fp));
