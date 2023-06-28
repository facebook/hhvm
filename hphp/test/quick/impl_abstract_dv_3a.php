<?hh
abstract class A  { abstract public function foo(int $x):mixed; }
interface      I  {                 function foo(int $x):mixed; }
abstract class B extends A implements I  { }
class C extends B {          public function foo(?AnyArray $x = null) :mixed{} }
<<__EntryPoint>> function main(): void {
$c = new C;
$c->foo(null);
echo "OK\n";
}
