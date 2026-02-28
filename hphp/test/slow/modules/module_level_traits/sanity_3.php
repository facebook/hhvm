<?hh

<<file:__EnableUnstableFeatures('module_level_traits')>>

trait T1 {
  public function foo(): void {
    echo "I am foo in T1\n";
  }
}

<<__ModuleLevelTrait>>
trait T2 {
  use T1;
}

class C {
  use T2;
}

<<__EntryPoint>>
function main(): void {
  include 'module_level_traits_module_a.inc';
  (new C())->foo();
}
