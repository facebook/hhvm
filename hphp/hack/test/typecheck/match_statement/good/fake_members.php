<?hh

<<file:__EnableUnstableFeatures('case_types')>>
<<file:__EnableUnstableFeatures('match_statements')>>

final class A {}
final class B {}
case type AB = A | B;

class ABBox {
  public function __construct(public AB $ab) {}
}

function test(ABBox $box): void {
  match ($box->ab) {
    _: A => hh_expect<A>($box->ab);
    _: B => hh_expect<B>($box->ab);
  }
}
