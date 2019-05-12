<?hh

class C {
  public function f() {
    var_dump("yep!");
  }
}

function f<reify T1>() {
  $x = function() {
    $c = new T1();
    $c->f();
  };
  $x();
}
<<__EntryPoint>> function main(): void {
f<C>();
}
