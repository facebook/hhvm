<?hh

<<file:__EnableUnstableFeatures('module_level_traits')>>

trait T1 {}

<<__ModuleLevelTrait>>
trait T2 {
  use T1;
}
