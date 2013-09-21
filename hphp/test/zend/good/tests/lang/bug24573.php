<?php

class Foo {
  function Bar() {
    $__this = $this;
    $this = null;
    debug_backtrace();
    $this = $__this;
  }
}

$f = new Foo;

$f->Bar();

echo "OK\n";

?>