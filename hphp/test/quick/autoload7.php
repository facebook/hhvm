<?php

function __autoload($a) {
  var_dump($a);
  if ($a == 'A') {
    class A {};
  }
}

function main() {
  $a = '\\A';
  new $a;
}

main();
