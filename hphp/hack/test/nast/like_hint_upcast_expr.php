<?hh
<<file:__EnableUnstableFeatures('upcast_expression')>>

class A {}

function like_hint_upcast_expr(mixed $x): void {
  $_ = $x upcast ~A;
}
