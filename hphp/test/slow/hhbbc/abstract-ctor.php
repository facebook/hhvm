<?hh

class another {}
abstract class SomethingElse {
  public function __construct() { return new another; }
}

class Something extends SomethingElse {}

function main() {
  $x = new Something(); // should infer the constructor returns Obj=another
}

main();
