<?php

class Foo {
  private $x = null;

  public function __get($k) { return array(1,2,3); }
  public function getter()  { return $this->x; }
  public function heh()     { unset($this->x); }
}

function main() {
  $f = new Foo;
  var_dump($f->getter());
  $f->heh();
  var_dump($f->getter());
}

main();
