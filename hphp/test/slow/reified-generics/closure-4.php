<?hh

class B {
  public function f() {
    var_dump("hellooooo");
  }
}

class C {
  public function f<reify T>() {
    var_dump("yep!");
    $b = new T();
    $b->f();
  }
}

function f<reify T1, reify T2>() {
  $x = () ==> {
  $y = () ==> {
    $c = new T1();
    $c->f<T2>();
  };
  $y();
  };
  $x();
}
<<__EntryPoint>> function main(): void {
f<C, B>();
}
