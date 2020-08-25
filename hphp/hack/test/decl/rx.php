<?hh // strict

class C {
  <<__Rx>>
  public function f1(Pure<(function(): int)> $f): void {}
  <<__Rx>>
  public function f2(Rx<(function(): int)> $f): void {}
  <<__Rx>>
  public function f3(RxShallow<(function(): int)> $f): void {}
  <<__Rx>>
  public function f4(RxLocal<(function(): int)> $f): void {}
}
