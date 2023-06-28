<?hh

function foo1<T as num>(inout T $x): void {
  $x = HH\Lib\Legacy_FIXME\cast_for_arithmetic($x) + 1;
}

function foo2<T as num>(inout T $x): void {
  if ($x > 0) $x = 'b';
}

function bar<T as num>(): T {
  return 'b';
}

type N = num;

function baz<T as N>(T $x) : T {
  return $x;
}

<<__EntryPoint>> function main() :mixed{
  $x = 'a';
  foo1(inout $x);
  foo2(inout $x);
  var_dump($x);
  var_dump(bar());
  var_dump(baz('c'));
}
