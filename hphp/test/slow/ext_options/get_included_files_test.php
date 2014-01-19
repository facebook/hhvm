<?php
// This file is included in the output.

require 'test3.inc';
include 'test1.inc';
include_once 'test2.inc';
// This should only be listed once in the output.
require 'test3.inc';
require_once 'test4.inc';

$included_files = get_included_files();
// Zend and HHVM output in different orders; level the field
sort($included_files);
foreach ($included_files as $filename) {
  $idx = strrpos($filename, "/");
  $pre = substr($filename, 0, $idx + 1);
  $suf = substr($filename, $idx + 1);
  var_dump($pre === dirname(__FILE__) . "/");
  var_dump($suf);
}
