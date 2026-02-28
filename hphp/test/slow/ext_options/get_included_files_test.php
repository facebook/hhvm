<?hh

function foo($files) :mixed{
  // Zend and HHVM output in different orders; level the field
  sort(inout $files);
  foreach ($files as $filename) {
    $idx = strrpos($filename, "/");
    $pre = substr($filename, 0, $idx + 1);
    $suf = substr($filename, $idx + 1);
    var_dump($pre === dirname(__FILE__) . "/");
    var_dump($suf);
  }
}

// This file is included in the output.

<<__EntryPoint>>
function main_get_included_files_test() :mixed{
require 'test3.inc';
include 'test1.inc';
include_once 'test2.inc';
// This should only be listed once in the output.
require 'test3.inc';
require_once 'test4.inc';
foo(get_included_files());
foo(get_required_files());
}
