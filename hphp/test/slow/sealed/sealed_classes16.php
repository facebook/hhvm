<?hh

<<__Sealed(SomeTrait::class)>>
interface SomeInterface {}

trait SomeTrait implements SomeInterface {}

trait SomeTrait2 {
  use SomeTrait;
}
<<__EntryPoint>> function main(): void { echo "Done.\n"; }
