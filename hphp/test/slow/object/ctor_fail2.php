<?php

function err($x, $y) { echo $y; echo "\n"; }
set_error_handler('err');
abstract class Asd {
  private function __construct() {}
}

function get_name() {
  apc_store('name', 'Asd'); // make invisible to static analysis
  return apc_fetch('name');
}

function x() { $name = get_name(); new $name(); } x();
