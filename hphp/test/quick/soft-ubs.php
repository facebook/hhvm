<?hh

function foo<T as int>(@T $x) : @T {
  return $x;
}

class Bar<T as int> {
  static public function baz(@?T $x) : @?T {
    return $x;
  }
}



<<__EntryPoint>>
function main() {
  var_dump(Bar::baz(null));
  var_dump(foo('a'));
  var_dump(Bar::baz('b'));
}
