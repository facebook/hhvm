<?hh

class C {
  public function f() :mixed{
    var_dump("yep!");
  }
}

function f<reify T1>() :mixed{
  $x = () ==> {
    $c = new T1();
    $c->f();
  };
  $x();
}
<<__EntryPoint>> function main(): void {
f<C>();
}
