<?hh // strict
class A {

  public function f8(): :cls {
    // OK
    return <cls />;
  }
}

class :cls extends XHPTest {
  public ?int $x;
}
