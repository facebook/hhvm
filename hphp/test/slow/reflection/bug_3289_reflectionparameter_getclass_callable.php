<?php

class Foo {
  public function bar(callable $baz) {}
}

var_dump((new ReflectionParameter(['Foo', 'bar'], 'baz'))->getClass());
