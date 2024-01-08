<?hh

<<file:__EnableUnstableFeatures('module_level_traits')>>

module MLT_A;

<<__ModuleLevelTrait>>
public trait T {
  internal function bar() { echo "bar in T\n"; }
}

class C {
  use T;

  public function foo() {
    $this->bar();
  }
}

<<__EntryPoint>>
function main() {
  (new C())->foo();
}
