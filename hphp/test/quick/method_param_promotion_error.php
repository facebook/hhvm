<?php

//
// only constructors can promote parameters
//
class A {
  public function f(protected $c) {}
}

