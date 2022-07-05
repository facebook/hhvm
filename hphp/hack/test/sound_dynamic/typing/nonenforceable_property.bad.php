<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

class C {
  private ?vec<int> $v;

  public function foo<T as supportdyn<mixed>>(~T $x) : ~T { return $x; }

  public function bar(?vec<int> $v): void {
    $this->v = $this->foo($v);
  }
}
