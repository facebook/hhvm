<?php

error_reporting(-1);

set_error_handler(function() {

    $GLOBALS['g']->p = '';
  });

class X {
  public $p = '';
  function foo() {
    $this->p->q->r->s = 1;
  }
}

$g = new X;
$g->foo();
var_dump($g);
