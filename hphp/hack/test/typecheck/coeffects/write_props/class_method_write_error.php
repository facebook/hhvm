<?hh

class Foo {

  public function __construct(
    public int $prop_int,
   ) {}

  public function pure_method()[] : void {
    $this->prop_int = 4; // Error
  }

}
