<?php

error_reporting(-1);

class Foo {
  function get() {
    return "bar";
  }
}

echo Foo::get();

function no_called() {
    Foo::get();
}
