<?hh
abstract class A  { abstract public function foo(int $x); }
class B extends A {          public function foo(int $x)   {} }
class C extends B {          public function foo(varray $x) {} }
<<__EntryPoint>> function main(): void {
$o = new C;
$o->foo(varray[]);
echo "OK\n";
}
