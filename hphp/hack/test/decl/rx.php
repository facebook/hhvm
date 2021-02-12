<?hh // strict

class C {
  <<__Pure>>
  public function f1(Pure<(function(): int)> $f): void {}
}
