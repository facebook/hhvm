<?hh

class A {
  function foo() :mixed{
    $cb = meth_caller('B', 'c');
    $cb(null);
  }
}

<<__EntryPoint>> function main(): void {
(new A())->foo();
}
