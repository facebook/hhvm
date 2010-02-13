<?php

class SplFileInfo {

  public function __construct($file_name) {
    hphp_splfileinfo___construct($this, $file_name);
  }

  public function getPath() {
    return hphp_splfileinfo_getpath($this);
  }

  public function getFilename() {
    return hphp_splfileinfo_getfilename($this);
  }

  public function getFileInfo($class_name = "") {
    return hphp_splfileinfo_getfileinfo($this, $class_name);
  }

  public function getBasename($suffix = "") {
    return hphp_splfileinfo_getbasename($this, $suffix);
  }

  public function getPathname() {
    return hphp_splfileinfo_getpathname($this);
  }  

  public function getPathInfo($class_name = "") {
    return hphp_splfileinfo_getpathinfo($this, $class_name);
  }

  public function getPerms() {
    return hphp_splfileinfo_getperms($this);
  }

  public function getInode() {
    return hphp_splfileinfo_getinode($this);
  }

  public function getSize() {
    return hphp_splfileinfo_getsize($this);
  }

  public function getOwner() {
    return hphp_splfileinfo_getowner($this);
  }

  public function getGroup() {
    return hphp_splfileinfo_getgroup($this);
  }

  public function getATime() {
    return hphp_splfileinfo_getatime($this);
  }

  public function getMTime() {
    return hphp_splfileinfo_getmtime($this);
  }

  public function getCTime() {
    return hphp_splfileinfo_getctime($this);
  }

  public function getType() {
    return hphp_splfileinfo_gettype($this);
  }

  public function isWritable() {
    return hphp_splfileinfo_iswritable($this);
  }

  public function isReadable() {
    return hphp_splfileinfo_isreadable($this);
  }

  public function isExecutable() {
    return hphp_splfileinfo_isexecutable($this);
  }

  public function isFile() {
    return hphp_splfileinfo_isfile($this);
  }

  public function isDir() {
    return hphp_splfileinfo_isdir($this);
  }

  public function isLink() {
    return hphp_splfileinfo_islink($this);
  }

  public function getLinkTarget() {
    return hphp_splfileinfo_getlinktarget($this);
  }

  public function getRealPath() {
    return hphp_splfileinfo_getrealpath($this);
  }
  
  public function __toString() {
    return hphp_splfileinfo___tostring($this);
  }

  public function openFile($mode = 'r', $use_include_path = false,
                           $context = null) {
    return hphp_splfileinfo_openfile($this, $mode,
                                     $use_include_path, $context);
  }

  public function setFileClass($class_name = "SplFileObject") {
    return hphp_splfileinfo_setfileclass($this, $class_name);
  }

  public function setInfoClass($class_name = "SplFileInfo") {
    return hphp_splfileinfo_setinfoclass($this, $class_name);
  }
}

class SplFileObject extends SplFileInfo implements RecursiveIterator,
  Traversable, Iterator, SeekableIterator {

  const DROP_NEW_LINE = 1;
  const READ_AHEAD = 2;
  const SKIP_EMPTY = 6;
  const READ_CSV = 8;

  public function __construct($filename, $open_mode = 'r',
                              $use_include_path = false,
                              $context = null) {
    hphp_splfileobject___construct($this, $file_name, $open_mode,
                                   $use_include_path, $context);
  }

  public function current() {
    return hphp_splfileobject_current($this);
  }

  public function eof() {
    return hphp_splfileobject_eof($this);
  }

  public function fflush() {
    return hphp_splfileobject_fflush($this);
  }

  public function fgetc() {
    return hphp_splfileobject_fgetc($this);
  }

  public function fgetcsv($delimiter = ",", $enclosure = "\"",
                           $escape = "\\") {
    return hphp_splfileobject_fgetcsv($this, $delimiter,
                                      $enclosure, $escape);
  }

  public function fgets() {
    return hphp_splfileobject_fgets($this);
  }

  public function fgetss($allowable_tags) {
    return hphp_splfileobject_fgetss($this, $allowable_tags);
  }

  public function flock($operation, &$wouldblock) {
    return hphp_splfileobject_flock($this, $wouldblock);
  }

  public function fpassthru() {
    return hphp_splfileobject_fpassthru($this);
  }

  public function fscanf($format) {
    return hphp_splfileobject_fscanf($this);
  }

  public function fseek($offset, $whence) {
    return hphp_splfileobject_fseek($this, $offset, $whence);
  }

  public function fstat() {
    return hphp_splfileobject_fstat($this);
  }

  public function ftell() {
    return hphp_splfileobject_ftell($this);
  }

  public function ftruncate($size) {
    return hphp_splfileobject_ftruncate($this, $size);
  }

  public function fwrite($str, $length) {
    return hphp_splfileobject_fwrite($this, $str, $length);
  }

  public function getChildren() {
    return null; // An SplFileOjbect does not have children
  }

  public function getCsvControl() {
    return hphp_splfileobject_getcvscontrol($this);
  }

  public function getFlags() {
    return hphp_splfileobject_getflags($this);
  }

  public function getMaxLineLen() {
    return hphp_splfileobject_getmaxlinelen($this);
  }

  public function hasChildren() {
    return false; // An SplFileOjbect does not have children
  }

  public function key() {
    return hphp_splfileobject_key($this);
  }

  public function next() {
    return hphp_splfileobject_next($this);
  }

  public function rewind() {
    return hphp_splfileobject_rewind($this);
  }

  public function seek($line_pos) {
    return hphp_splfileobject_seek($this, $line_pos);
  }

  public function setCsvControl($delimiter = ",", $enclosure = "\"",
                                $escape = "\\") {
    return hphp_splfileobject_setcsvcontrol($this, $delimiter, $enclosure,
                                            $escape); 
  }

  public function setFlags($flags) {
    return hphp_splfileobject_setflags($this, $flags);
  }

  public function setMaxLineLen($max_len) {
    return hphp_splfileobject_setmaxlinelen($this, $max_len);
  }

  public function valid() {
    return hphp_splfileobject_valid($this);
  }
}
