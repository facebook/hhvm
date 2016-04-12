<?php

// Ensure proper inheritance with get_class(anon class instance) used via
// class_alias

class_alias(get_class(new class { protected $foo = 1; }), "AnonBase");
var_dump((new class extends AnonBase {
  function getFoo() {
    return $this->foo;
  }
})->getFoo());
?>
