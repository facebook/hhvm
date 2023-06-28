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

class D<reify T> {
  function f<reify T1>() :mixed{
    $x = () ==> {
      $c = new T1();
      $c->f<T>();
    };
    $x();
  }
}
<<__EntryPoint>> function main(): void {
$d = new D<B>();
$d->f<C>();
}
