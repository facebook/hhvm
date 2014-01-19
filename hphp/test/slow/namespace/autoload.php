<?php

namespace {
  function __autoload($a) {
    var_dump($a);
    if ($a == 'A') {
      class A {};
    }
  }
}

namespace A {
  $a = '\\A';
  new $a;
}
