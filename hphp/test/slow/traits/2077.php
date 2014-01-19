<?php

trait baz  {
  public function bar() {
 yield 1;
 }
}
class foo {
  use baz;
  public function bar() {
}
}
