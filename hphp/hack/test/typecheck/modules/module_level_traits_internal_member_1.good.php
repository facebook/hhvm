//// a.php
<?hh

new module A {}

//// b.php
<?hh

<<file: __EnableUnstableFeatures('module_level_traits')>>

module A;

<<__ModuleLevelTrait>>
trait T {
  internal function foo(): void { echo "foo in T\n"; }
}

class C { use T; }

<<__EntryPoint>>
function main(): void {
  (new C())->foo();
}
