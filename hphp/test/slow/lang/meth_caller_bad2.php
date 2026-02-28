<?hh

class A {
  function foo() :mixed{
    $cb = meth_caller(B::class, 'c');
    $cb(null);
  }
}

<<__EntryPoint>> function main(): void {
(new A())->foo();
}
