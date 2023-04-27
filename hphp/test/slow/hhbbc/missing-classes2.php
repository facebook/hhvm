<?hh

trait T2 {
  public function meth3(Bad5 $a) {
    return (Bad6 $x) ==> 123;
  }
}

trait T {
  use T2;
  public function meth1(Bad2 $a) {
    return (Bad4 $x) ==> 123;
  }
}

class Bad1 extends T {}
class Bad2 extends T {}
class Bad3 extends T {}
class Bad4 extends T {}
class Bad5 extends T {}
class Bad6 extends T {}

class A {
  use T;
  public function meth2(Bad1 $a) {
    return (Bad3 $x) ==> 123;
  }
}

class B extends A {}

function func1(Bad1 $a) {
  var_dump($a);
}

<<__EntryPoint>>
function main() {
  func1(new Bad1());
}
