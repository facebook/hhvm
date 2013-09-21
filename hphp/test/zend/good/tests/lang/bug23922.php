<?php
  class foo
  {
    public $foo = 1;

    function as_string()
    { assert('$this->foo == 1'); }

    function as_expr()
    { assert($this->foo == 1); }
  }

  $foo = new foo();
  $foo->as_expr();
  $foo->as_string();
?>