<?php

class dtor {
  private $i;
  function __construct($i) { $this->i = $i; }
  function __destruct() { echo "dtor: $this->i\n"; }
}

class X {
  public $decl;
}

function go() {
  $x = (new X);
  $x->decl = new dtor(0);
  $x->dyn = new dtor(1);
}

go();
