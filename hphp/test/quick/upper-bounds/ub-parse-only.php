<?hh

function foo1<T as int>(T $x) : T {
  return $x;
}

function foo2<T as int as string>(T $x) : T {
  return $x;
}

class Bar<T as int> {
  public static function foo3(T $x) : T {
    return $x;
  }
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(foo1('a'));
  var_dump(foo2(vec[1, 2]));
  var_dump(Bar::foo3('a'));
}
