<?hh
class Foo implements Serializable {
  public function serialize() :mixed{ return "foobar"; }
  public function unserialize($str) :mixed{ }
}

class Baz {}

<<__EntryPoint>>
function main_with_nonexistant_serializable_class() :mixed{
$f = new Foo(42);
$fs = serialize($f);
$non_existent_bar = preg_replace('/Foo/', 'Bar', $fs);
var_dump( unserialize($non_existent_bar) );
$non_serializable_baz = preg_replace('/Foo/', 'Baz', $fs);
var_dump( unserialize($non_serializable_baz) );
}
