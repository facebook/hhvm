<?php


trait T {
  private function bar() {}
  public function foo() {}
}

class A {
  use T;
}

print_r(get_class_methods('A'));
