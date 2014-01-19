<?php

class B {
  protected $p;
}

class C extends B {}

function main() {
  $o = new C;
  var_dump($o->p);
}

main();
