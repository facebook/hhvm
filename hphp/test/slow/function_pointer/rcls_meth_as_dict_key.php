<?hh

final class Test {
  public static function foo<reify T>(): void {}
}

<<__EntryPoint>>
function main(): void {
  $d = dict[];
  $f = Test::foo<int>;
  try {
    $d[$f];
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
