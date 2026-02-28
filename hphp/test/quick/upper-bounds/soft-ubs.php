<?hh

function foo<T as int>(<<__Soft>> T $x) : <<__Soft>> T {
  return $x;
}

class Bar<T as int> {
  <<__Soft>>
  public T $x = 'a';
  static public function baz(<<__Soft>> ?T $x) : <<__Soft>> ?T {
    return $x;
  }
}



<<__EntryPoint>>
function main() :mixed{
  var_dump(Bar::baz(null));
  var_dump(foo('a'));
  var_dump(Bar::baz('b'));
  $o = new Bar;
  $o->x = 3.14;
  var_dump($o->x);
}
