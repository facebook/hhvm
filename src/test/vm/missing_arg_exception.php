<?php
set_error_handler('handler');
function handler() { throw new Exception; }
function foo(&$r) {}
function test() {
  try {
    foo();
  } catch (Exception $e) {
  }
  var_dump('ok');
}
test();
