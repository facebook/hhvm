<?hh

interface I { function foo(); }
interface J extends I {}
interface K { function bar(); }
abstract class B implements J, K {}
abstract class C extends B implements K {}
class D extends C {}

<<__EntryPoint>>
function main(): void {
  echo "Done\n";
}
