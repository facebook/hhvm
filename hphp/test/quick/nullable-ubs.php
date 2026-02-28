<?hh

function foo<T as int>(<<__Soft>>?T $x) : <<__Soft>>?T {
  return $x;
}

class Bar<T as int> {
  static public function baz(<<__Soft>>?T $x) : <<__Soft>>?T {
    return $x;
  }
}

type Baz = ?int;
function foobar<T as ?Baz>(<<__Soft>>?T $x) : <<__Soft>>?T {
  return $x;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo(null));
  var_dump(Bar::baz(null));
  var_dump(foobar(null));
  var_dump(foobar(1));
  var_dump(foo('a'));
  var_dump(Bar::baz('b'));
  var_dump(foobar('c'));
}
