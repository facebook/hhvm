<?php

//
// field redeclaration, error
//
class A {
  public $c;
  public function __construct(protected $c) {}
}

