//// module_A.php
<?hh
new module A {}

//// A.php
<?hh

module A;

<<file: __EnableUnstableFeatures('module_level_traits')>>

// module level trait can use public non-module level trait

trait T1 {}

<<__ModuleLevelTrait>>
trait T2 {
  use T1;
}
