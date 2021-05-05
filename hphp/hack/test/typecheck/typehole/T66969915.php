<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

class Bug {
  private string $a;
  public function __construct() {
    $this->takes_string($this->a);
    $this->a = '';
  }
  private function takes_string(string $_): void {}
}
<<__EntryPoint>>
function main(): void {
  new Bug();
}
