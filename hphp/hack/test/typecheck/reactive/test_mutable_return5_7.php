<?hh // strict
class A {

  public function f8(): :cls {
    // OK
    return <cls />;
  }
}

class :cls {
  public ?int $x;
}
