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

class D<reify T> {
  function f<reify T1>() {
    $x = () ==> {
      $c = new T1<reify T>();
      $c->f();
    };
    $x();
  }
}

$d = new D<reify B>();
$d->f<reify C>();
