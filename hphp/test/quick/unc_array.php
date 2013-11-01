<?php

class Foo {
  public static function blah() { return true; }
}

function what($k) {}

function a() {
  $x = Foo::blah() ? array(1,2,3) : null;
  what($x);
}

a();
