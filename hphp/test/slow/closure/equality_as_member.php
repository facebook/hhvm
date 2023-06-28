<?hh

class Foo {
  public function __construct() {
    $this->foo = function() { var_dump('herpderp'); };
  }
}


<<__EntryPoint>>
function main_equality_as_member() :mixed{
var_dump((new Foo()) == (new Foo()));
}
