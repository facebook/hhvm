<<file:__EnableUnstableFeatures('upcast_expression')>>
<?hh

class A {}
class B<T> {}

function wildcard_hint_upcast_expr(A $x): void {
  $_ = $x upcast B<_>;
}
