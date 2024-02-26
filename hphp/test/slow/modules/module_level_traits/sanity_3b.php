<?hh

<<file:__EnableUnstableFeatures('module_level_traits')>>

module MLT_A;

<<__ModuleLevelTrait>>
trait T1 {
  internal int $x = 42;
}

class C {
  use T1;
}

<<__EntryPoint>>
function main(): void {
  include 'module_level_traits_module_a.inc';
  echo (new C())->x;
  echo " done\n";
}
