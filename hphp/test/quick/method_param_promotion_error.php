<?hh

//
// only constructors can promote parameters
//
class A {
  public function f(protected $c) {}
}

<<__EntryPoint>> function main(): void {}
