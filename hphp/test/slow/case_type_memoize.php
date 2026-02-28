<?hh
<<file:__EnableUnstableFeatures('case_types')>>

class C {}
class D implements IMemoizeParam {
  public function __construct(private string $x) {}
  public function getInstanceKey(): string { return $this->x; }
}

case type Good = int | bool | IMemoizeParam;

case type Bad = int | bool | C;

<<__Memoize>>
function f(Good $x): void { var_dump($x); }

<<__Memoize>>
function g(Bad $x): void { var_dump($x); }

<<__EntryPoint>>
function main() {
  f(10); f(12);
  f(10); f(12);

  f(new D("hi")); f(new D("bye"));
  f(new D("hi")); f(new D("bye"));

  g(true); g(false);
  g(true); g(false);

  g(new C);
}
