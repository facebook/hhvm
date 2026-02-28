<?hh

/**
 * For some of the tests below the VM produces different output than
 * Zend. This is because Zend is more liberal about passing in the
 * current instance to the callee. Zend is in the process of deprecating
 * this behavior.
 */

class D5 {
  private function foo() :mixed{
    echo "D5::foo " . (isset($this) ? "true\n" : "false\n");
  }
  public function test() :mixed{
    call_user_func('F5::foo');
    call_user_func(vec['F5','foo']);
  }
}
class E5 extends D5 {
}
class X5 extends D5 {
  private function foo() :mixed{
    echo "X5::foo\n";
  }
}
class F5 extends X5 {
}

function main5() :mixed{
  $e5 = new E5;
  $e5->test(); // Fatals
}
<<__EntryPoint>> function main(): void {
main5();
}
