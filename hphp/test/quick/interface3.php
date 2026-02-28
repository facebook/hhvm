<?hh
interface I { function foo():mixed; }
abstract class B implements I {}
abstract class C extends B {}
class D extends C {}
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
