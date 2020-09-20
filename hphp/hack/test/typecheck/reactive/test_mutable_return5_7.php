<?hh // strict

class A {
  <<__Rx, __MutableReturn>>
  public function f8(): :cls {
    // OK
    return <cls />;
  }
}

class :cls {
  public ?int $x;
}
