<?hh
abstract class A  { abstract public function foo(int $x); }
interface      I  {                 function foo(int $x); }
abstract class B extends A implements I  { }
class C extends B {          public function foo(arraylike $x = null) {} }
<<__EntryPoint>> function main(): void {
$c = new C;
$c->foo(null);
echo "OK\n";
}
