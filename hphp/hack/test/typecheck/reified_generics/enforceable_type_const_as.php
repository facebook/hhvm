<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

abstract class Base {}

abstract class A {
  <<__Enforceable>>
  abstract const type TE as Base;

  abstract const type TNotE as Base;

  public function f(): void {
    3 as this::TE;
    4 as this::TNotE;
  }
}
