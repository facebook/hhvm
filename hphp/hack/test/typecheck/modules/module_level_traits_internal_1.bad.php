//// module_A.php
<?hh
new module A {}

//// A.php
<?hh

module A;

<<file: __EnableUnstableFeatures('module_level_traits')>>

<<__ModuleLevelTrait>>
internal trait T {}
