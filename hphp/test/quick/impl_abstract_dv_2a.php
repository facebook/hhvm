<?hh
abstract class A  { abstract public function foo(int $x):mixed; }
class B extends A {          public function foo(int $x)   :mixed{} }
class C extends B {          public function foo(varray $x) :mixed{} }
<<__EntryPoint>> function main(): void {
$o = new C;
$o->foo(vec[]);
echo "OK\n";
}
