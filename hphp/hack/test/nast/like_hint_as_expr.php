<?hh
class C {}

function like_hint_as_expr(mixed $x): bool {
  $_ = $x as ~C;
  return false;
}
