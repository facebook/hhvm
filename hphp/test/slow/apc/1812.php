<?php

function foo(&$ref, $a) {
  $ref = $a;
  print_r($a);
  apc_store('table', $a);
}

<<__EntryPoint>>
function main() {
  $a = array();
  foo(&$a[], $a);
}
