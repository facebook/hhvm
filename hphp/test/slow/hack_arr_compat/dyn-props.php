<?php

class C {}

class D {
  private $array;

  function __construct() {
    $this->array = array();
  }

  function __get($key) {
    $this->array[$key] = $this->array[$key] ?? null;
    return $this->array[$key];
  }
}

function main() {
  $c = new C();
  $c->foo[] = 42;
  $c->bar += 42;
  $c->baz++;
  var_dump($c);

  $d = new D();
  $d->bar += 42;
  $d->baz++;
  var_dump($d);
}

main();
