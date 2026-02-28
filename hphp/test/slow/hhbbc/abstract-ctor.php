<?hh

class another {}
abstract class SomethingElse {
  public function __construct() { return new another; }
}

class Something extends SomethingElse {}

function main() :mixed{
  $x = new Something(); // should infer the constructor returns Obj=another
}


<<__EntryPoint>>
function main_abstract_ctor() :mixed{
main();
}
