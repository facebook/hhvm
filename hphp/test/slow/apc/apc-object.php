<?php

if (apc_exists('foo')) {
  class X {
    private $foo;
    function __construct() { $this->foo='bar'; }
    function show() { var_dump($this->foo); }
  };

  var_dump(apc_fetch('foo'));
} else {
  class X {
    private $foo;
    function __construct() { $this->foo='bar'; }
    function show() { var_dump($this->foo); }
  }
  apc_store('foo', new X);
  var_dump(apc_fetch('foo'));
}
