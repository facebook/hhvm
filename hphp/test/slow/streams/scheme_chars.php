<?php

function main() {
  var_dump(file_exists('this+is-a.valid123scheme://example.com'));
  var_dump(file_exists('this+is_not_a.valid123scheme://example.com'));
  var_dump(file_exists('/this+is-not-a.valid123scheme://example.com'));
  var_dump(file_exists('this.is.a.valid.scheme://example.com'));

  $e_acute = chr(130);
  var_dump(file_exists($e_acute.'this.is_not.a.valid.scheme://example.com'));
}
main();
