<?php

function t() {
  if (mt_rand()) {
    class Foo {
      const VALUE = __CLASS__ . '::VALUE';
    }
  } else {
    class Foo {
      const VALUE = __CLASS__ . '::VALUE';
    }
  }
}

t();
var_dump(Foo::VALUE);
