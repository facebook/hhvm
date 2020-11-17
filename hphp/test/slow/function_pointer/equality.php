<?hh

function foo<reify T>(): void {}

function bar<reify T>(): void {}

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) {
  wrap(() ==> var_dump($x == $y));
  wrap(() ==> var_dump($x === $y));
  wrap(() ==> var_dump($x != $y));
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
