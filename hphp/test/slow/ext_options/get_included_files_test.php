<?php
// This file is included in the output.

require 'test3.inc';
include 'test1.inc';
include_once 'test2.inc';
// This should only be listed once in the output.
require 'test3.inc';
require_once 'test4.inc';

function foo($files) {
  // Zend and HHVM output in different orders; level the field
  sort($files);
  foreach ($files as $filename) {
    $idx = strrpos($filename, "/");
    $pre = substr($filename, 0, $idx + 1);
    $suf = substr($filename, $idx + 1);
    var_dump($pre === dirname(__FILE__) . "/");
    var_dump($suf);
  }
}
foo(get_included_files());
foo(get_required_files());
