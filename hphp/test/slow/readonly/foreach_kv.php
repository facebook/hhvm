<?hh
class Bar {}
class Foo {
  public int $prop;
  public readonly Bar $ro;
  public Bar $not_ro;
  public function __construct() {
    $this->prop = 1;
    $this->ro = new Bar();
    $this->not_ro = new Bar();
  }
}


function test(readonly dict<int,Foo> $vec, bool $b): int {
  foreach($vec as $key => $val) {
    if($b) {
      return $key; // this is okay
    }
    $val->prop = 3; // error, $val is readonly
  }
}
