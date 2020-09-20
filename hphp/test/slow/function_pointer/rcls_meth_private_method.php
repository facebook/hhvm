<?hh

final class Test {
  private static function notPublic<reify T>(): void {}
}

<<__EntryPoint>>
function main(): void {
  $a = Test::notPublic<int>;
  var_dump($a);
}
