<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(): C {
  return new C();
}

<<__Rx, __MutableReturn>>
function make2(int $a): C {
  return new C();
}

class A {
  <<__Rx, __MutableReturn>>
  public function f1(): A {
    // OK - returns fresh object
    return new A();
  }

  <<__Rx, __MutableReturn>>
  public function f2(): A {
    // OK - returns result of another mutable returning method
    return $this->f1();
  }

  <<__Rx, __MutableReturn>>
  public function f3(): C {
    // OK - returns result of another mutable returning function
    return make();
  }

  <<__Rx, __MutableReturn>>
  public function f4(): C {
    // OK - returns mutably owned local
    $a = \HH\Rx\mutable(make());
    return $a;
  }

  <<__Rx, __MutableReturn>>
  public function f7(): C {
    // OK - pipe where right hand side returns mutably owned value
    return 1 |> \HH\Rx\mutable(make2($$));
  }

  <<__Rx, __MutableReturn>>
  public function f8(): :cls {
    // OK - XHP literal is considered mutably owned
    return <cls />;
  }
}

class :cls {
  public ?int $x;
}
