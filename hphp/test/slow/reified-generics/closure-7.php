<?hh

class C {
  public function f() :mixed{
    var_dump("yep!");
  }
}

function f<reify T1>() :mixed{
  $a = 1;
  $x = function() use ($a) {
    $c = new T1();
    $c->f();
  };
  $x();
}
<<__EntryPoint>> function main(): void {
f<C>();
}
