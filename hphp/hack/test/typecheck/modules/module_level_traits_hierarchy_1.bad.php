//// module_A.php
<?hh
new module A {}

//// A.php
<?hh

module A;

<<file: __EnableUnstableFeatures('module_level_traits')>>


// a module level trait cannot use a non-module level trait

trait T1 {}

<<__ModuleLevelTrait>>
trait T2 {
  use T1;
}
