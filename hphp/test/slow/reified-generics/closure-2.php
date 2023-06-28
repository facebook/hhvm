<?hh

class B {
  public function f() :mixed{
    var_dump("hellooooo");
  }
}

class C {
  public function f<reify T>() :mixed{
    var_dump("yep!");
    $b = new T();
    $b->f();
  }
}

function f<reify T1, reify T2>() :mixed{
  $x = () ==> {
    $c = new T1();
    $c->f<T2>();
  };
  $x();
}
<<__EntryPoint>> function main(): void {
f<C, B>();
}
