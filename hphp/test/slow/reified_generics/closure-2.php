<?hh

class B {
  public function f() {
    var_dump("hellooooo");
  }
}

class C<reify T> {
  public function f() {
    var_dump("yep!");
    $b = new T();
    $b->f();
  }
}

function f<reify T1, reify T2>() {
  $x = () ==> {
    $c = new T1<reify T2>();
    $c->f();
  };
  $x();
}

f<reify C, reify B>();
