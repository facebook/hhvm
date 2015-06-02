<?php
set_error_handler('handler');
function handler() {
  throw new Exception;
}
function test($x) {}
function main($a) {
  try {
    test(...$a);
  } catch (Exception $e) {}
}
main(new stdclass);
echo "OK\n";
