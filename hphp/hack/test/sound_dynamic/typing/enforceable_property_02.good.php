<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

<<__SupportDynamicType>>
class C {
  private ?int $v;

  public function foo(?int $x): void {}

  public function bar(): void {
    $this->foo($this->v);
  }
}
