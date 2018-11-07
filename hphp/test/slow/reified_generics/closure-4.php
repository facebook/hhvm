<?hh

class B {
  public function f() {
    var_dump("hellooooo");
  }
}

class C<reified T> {
  public function f() {
    var_dump("yep!");
    $b = new T();
    $b->f();
  }
}

function f<reified T1, reified T2>() {
  $x = () ==> {
  $y = () ==> {
    $c = new T1<reified T2>();
    $c->f();
  };
  $y();
  };
  $x();
}

f<reified C, reified B>();
