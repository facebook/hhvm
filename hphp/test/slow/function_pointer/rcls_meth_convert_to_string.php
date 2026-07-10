<?hh

final class Test {
  public static function foo<reify T>(): void {}
}

<<__EntryPoint>>
function main(): void {
  $f = Test::foo<int>;
  try {
    $g = (string)$f;
    var_dump($g);
  } catch (Exception $e) {
    echo $e->__toString()."\n";
  } finally {}
}
