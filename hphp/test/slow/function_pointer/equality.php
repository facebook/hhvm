<?hh

function foo<reify T>(): void {}

function bar<reify T>(): void {}

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) :mixed{
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\eq($x, $y)));
  wrap(() ==> var_dump($x === $y));
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\neq($x, $y)));
  wrap(() ==> var_dump($x !== $y));
  print("\n");
}

<<__EntryPoint>>
function main(): void {
  comp(foo<int>, foo<int>);

  comp(foo<int>, foo<string>);

  comp(foo<int>, bar<int>);

  comp(foo<int>, 'foo');

  comp(foo<int>, foo<>);

  comp(foo<int>, true);

  comp(foo<int>, 1);

  comp(foo<int>, vec[]);
}
