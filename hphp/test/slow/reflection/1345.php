<?php

trait T {
}
interface I {
}

<<__EntryPoint>>
function main_1345() {
foreach (get_declared_classes() as $c) {
  if ($c == 'T' || $c == 'I') {
    var_dump('failed');
    exit(0);
  }
}
var_dump('OK!');
}
