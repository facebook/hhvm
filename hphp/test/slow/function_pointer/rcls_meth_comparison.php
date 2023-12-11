<?hh

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) :mixed{
  wrap(() ==> var_dump($x < $y));
  wrap(() ==> var_dump($x <= $y));
  wrap(() ==> var_dump($x > $y));
  wrap(() ==> var_dump($x >= $y));
  wrap(() ==> var_dump($x <=> $y));
  print("\n");
}

final class Test {
  public static function foo<reify T>(): void {}
}

<<__EntryPoint>>
function main(): void {
  $f = Test::foo<int>;

  comp($f, $f);

  comp($f, true);

  comp($f, 1);

  comp($f, vec['Test','foo']);

  comp($f, Test::foo<>);

  comp($f, "Test::foo");
}
