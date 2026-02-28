<?hh

<<file:__EnableUnstableFeatures('module_level_traits_extensions')>>

module MLT_A;

<<__ModuleLevelTrait>>
public trait T {
  internal function bar() { echo "bar in T\n"; }
}

public trait T1 {
  use T;
}


class C {
  use T1;

  public function foo() {
    $this->bar();
  }
}

<<__EntryPoint>>
function main() {
  (new C())->foo();
}
