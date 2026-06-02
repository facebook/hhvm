<?hh
class WWWTest {}
class InternalClassTest extends WWWTest {
  public function test(): void {
    $f = InternalTarget::greet<>;
    var_dump($f());
  }
}

<<__EntryPoint>>
function main(): void {
  include "internal-class.module_a.inc";
  include "internal-class.inc";
  (new InternalClassTest())->test();
}
