<?hh

final class Test {
  public function notStatic<reify T>(): void {}
}

<<__EntryPoint>>
function main(): void {
  try {
    $a = Test::notStatic<int>;
  } catch (Exception $e) {
    echo $e->getMessage();
  }
}
