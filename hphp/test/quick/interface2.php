<?hh
interface I { public function foo(); }
interface J extends I {}
interface K extends I {}
interface L extends J, K {}
class C implements K { public function foo() {} }
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
