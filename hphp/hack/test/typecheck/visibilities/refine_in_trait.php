<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

trait TR {
  private bool $fld = false;
  final public function foo(): void {
    invariant($this is C, "For hack");
    $this->fld = true;
  }
}

abstract class C {
  use TR;
}
