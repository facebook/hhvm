<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function expect_int(int $_): void {}

class C {
  private ?Set<string> $s = null;

  private function test(): void {
    $_ = $this->s ?? varray[''];
    expect_int($this->s);
  }
}
