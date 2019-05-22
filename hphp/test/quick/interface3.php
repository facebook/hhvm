<?hh
interface I { function foo(); }
abstract class B implements I {}
abstract class C extends B {}
class D extends C {}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
