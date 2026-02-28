<?hh

class C {
  private static function foo(): void {
    echo("C::foo\n");
  }
  public function testit(D $d): void {
    $d::foo();
  }
}
final class D extends C {
  private static function foo(): void {
    echo("D::foo\n");
  }
}
<<__EntryPoint>>
function main(): void {
  $d = new D();
  $d->testit($d);
}
