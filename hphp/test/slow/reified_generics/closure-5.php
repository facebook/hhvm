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

class D<reify T> {
  function f<reify T1>() {
    $x = () ==> {
    $y = () ==> {
      $c = new T1();
      $c->f<reify T>();
    };
    $y();
    };
    $x();
  }
}

$d = new D<reify B>();
$d->f<reify C>();
