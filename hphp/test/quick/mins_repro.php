<?hh
// This file test is useful because the HHAS
// has compiler-inserted property names prefixed with 0:
//...
// .class {} [no_override] Closure$a::b (10,11) extends Closure {
//   .property [private sys_initial_val] <"" N > 0splice1 =
//     uninit;
//...
// And this phenomena previously had no coverage in hphp/test

final class a {
  public function c(): void {
    $this->b();
  }
  private function b(): ?NS<NSBool> {
    return Nonsense`
      ${Mins::fromLispy($c)} &&
      ${$a}
    `;
  }

}
<<__EntryPoint>>
function main_entry(): void {
  $d = new a;
  try {
    $d->c();
  } catch (Exception $e) {
    var_dump('mins test');
  }
}
