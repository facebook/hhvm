<?hh // strict
interface RxA {
}

class C {
  <<__Rx, __OnlyRxIfImpl(RxA::class)>>
  public function f(C $c): void {
    // ERROR, condition not matched
    // call to this method is permitted in reactive context
    // if receiver implements RxA and it does not force type of C $c to
    // implement RxA - as a result if receiver implements RxA and $c does not -
    // it will be able to invoke non-reactive method in reactive contet
    $c->g();
  }
  <<__Rx, __OnlyRxIfImpl(RxA::class)>>
  public function g(): void {
  }
}
