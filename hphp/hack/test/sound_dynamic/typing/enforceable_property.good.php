<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  private ?int $v;

  public function foo<T as supportdyn<mixed>>(~T $x) : ~T { return $x; }

  public function bar(?int $v): void {
    $this->v = $this->foo($v);
  }
}
