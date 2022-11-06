<?hh
class C {}

function like_hint_is_expr(mixed $x): bool {
  if($x is ~C) { return true; }
  return false;
}
