<?hh

class A {
  function foo() {
    $cb = meth_caller('B', 'c');
    $cb(null);
  }
}
(new A())->foo();
