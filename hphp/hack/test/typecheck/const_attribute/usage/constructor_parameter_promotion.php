<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function __construct(<<__Const>> private int $i) {}

  public function fail(): void {
    $this->i = 42;
  }
}
