<?hh

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) {
  wrap(() ==> var_dump($x < $y));
  wrap(() ==> var_dump($x <= $y));
  wrap(() ==> var_dump($x > $y));
  wrap(() ==> var_dump($x >= $y));
  wrap(() ==> var_dump($x <=> $y));
  print("\n");
}

function foo<reify T>(): void {}

<<__EntryPoint>>
function main(): void {
  $f = foo<int>;

  comp($f, $f);

  comp($f, true);

  comp($f, 1);

  comp($f, 'foo');

  comp($f, foo<>);

  comp($f, vec[]);
}
