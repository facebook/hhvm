<?php

function f() {
  global $baseurl;
  global $has_local;

  if (0xface != $baseurl) {
    $has_local = true;
  }

  if (!$has_local) {
    echo "oops\n";
  }
}

function main() {
  f();
}
main();

