<?php

/**
 * The purpose of this test is to make sure that '/tmp/foo/http://example.com'
 * is treated as a file path, not a URI with a scheme of
 * /tmp/foo/http - if it is, an unknown stream type warning will be raised.
 */
function main() {
  $root = tempnam('/tmp', 'hhvmtest');
  unlink($root);
  mkdir($root);
  $fnam = $root.'/http://example.com';
  var_dump(file_exists($fnam));
  mkdir($root.'/http:');
  file_put_contents($fnam, 'foobar');
  var_dump(file_exists($fnam));
  var_dump(file_get_contents($fnam));
  unlink($fnam);
  rmdir($root.'/http:');
  rmdir($root);
}

main();
