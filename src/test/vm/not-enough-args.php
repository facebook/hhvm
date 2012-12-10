<?php

function one($a) { echo "one\n"; }
function two($a, $b) { echo "two\n"; }
function three($a, $b, $c) { echo "three\n"; }

function error_handler($errno, $errstr) {
  throw new Exception("$errno, $errstr");
}

function main() {
  one(1);
  two(1);
  three(1);
  set_error_handler('error_handler');
  one(1);
  two(1);
  three(1);
}
main();
