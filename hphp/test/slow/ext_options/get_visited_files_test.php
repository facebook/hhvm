<?hh

function foo($files) {
  if (!$files) {
    echo "Empty\n";
    return;
  }
  $files = vec($files);
  sort(inout $files);
  foreach ($files as $filename) {
    $idx = strrpos($filename, "/");
    $pre = substr($filename, 0, $idx + 1);
    $suf = substr($filename, $idx + 1);
    var_dump($pre === dirname(__FILE__) . "/");
    var_dump($suf);
  }
}

// This file is not included in the output.

<<__EntryPoint>>
function main_get_included_files_test() {
// Test when it's not turned on
include 'test1.inc';
foo(get_visited_files());

// Test when it's empty
record_visited_files();
foo(get_visited_files());
require 'test3.inc';
include 'test1.inc';
include_once 'test2.inc';
// This should only be listed once in the output.
require 'test3.inc';
test1();
require_once 'test4.inc';
foo(get_visited_files());
}
