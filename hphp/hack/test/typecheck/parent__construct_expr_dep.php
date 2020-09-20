<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

interface I {}

abstract class A<T as I> {
  public function __construct(?T $_) {}
}

class B<T as I> extends A<T> {
  <<__Override>>
  public function __construct(?T $x = null) {
    parent::__construct($x);
  }
}
