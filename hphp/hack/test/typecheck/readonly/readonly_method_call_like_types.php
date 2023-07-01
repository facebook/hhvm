<?hh
class Foo {
  public function hello(): string {
    return "";
  }
}
function test(): void {
  $x = new Foo();
  $readonly_x = readonly $x;
  $x->hello();
  $readonly_x->hello(); // error
  $f = () ==> {
    $x->hello(); // no error
    $readonly_x->hello(); // error
  };
  $f();
  $f = readonly () ==> {
    $x->hello(); // error
    $readonly_x->hello(); // error
  };
  $f();
}
