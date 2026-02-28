<?hh

final class Test {
  public static function foo<reify T>(): void {}
}

<<__EntryPoint>>
function main(): void {
  $f = Test::foo<int>;
  try {
    $g = (int)$f;
    echo $g;
  } catch (Exception $e) {
    echo $e."\n";
  } finally {}
}
