<?hh

class Bar {
  public static function baz<reify T>(): void {}
}

<<__EntryPoint>>
function test(): void {
  $y = vec[Bar::baz<int>];
  var_dump($y);
  print_r($y);
}
