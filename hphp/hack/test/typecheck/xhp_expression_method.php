<?hh // partial
final class :x extends XHPTest {
  public function foo(): string {
    return "";
  }
}


function test(): string {
  return <x></x>->foo();
}
