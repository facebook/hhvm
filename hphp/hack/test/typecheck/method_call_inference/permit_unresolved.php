<?hh

class Box<T> {
  public function get() : T {
    throw new Exception();
  }
}

function test1() : void {
  $box = new Box();
  $unresolved = $box->get();
  $unresolved->foo(); // OK
}

function test2() : void {
  $map = Map{};
  $entry = $map["foo"];
  $entry->bar(); // OK
}
