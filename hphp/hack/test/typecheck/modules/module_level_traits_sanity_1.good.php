//// module_A.php
<?hh
new module A {}

//// A.php
<?hh

module A;

<<__ModuleLevelTrait>>
trait T1 {}

//// B.php
<?hh

// this is redundant
<<file: __EnableUnstableFeatures('module_level_traits')>>

module A;

<<__ModuleLevelTrait>>
trait T2 {}
