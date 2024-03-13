<?hh

module A;

<<__ModuleLevelTrait>>
trait T {
  internal function foo(): void { echo "foo in T\n"; }
}
