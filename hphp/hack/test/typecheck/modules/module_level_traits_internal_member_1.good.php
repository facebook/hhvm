//// a.php
<?hh

new module A {}

//// b.php
<?hh

module A;

<<file:__EnableUnstableFeatures('module_level_traits_extensions')>>

<<__ModuleLevelTrait>>
trait T {
  internal function foo(): void { echo "foo in T\n"; }
}

class C { use T; }

<<__EntryPoint>>
function main(): void {
  (new C())->foo();
}
