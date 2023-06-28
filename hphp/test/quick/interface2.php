<?hh
interface I { public function foo():mixed; }
interface J extends I {}
interface K extends I {}
interface L extends J, K {}
class C implements K { public function foo() :mixed{} }
<<__EntryPoint>> function main(): void {
echo "Done\n";
}
