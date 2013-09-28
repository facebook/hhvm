<?php

class a {
  protected $foo = 10;
}
$x = new a;
apc_store('x', array($x));
$x = apc_fetch('x');
var_dump($x[0]);
