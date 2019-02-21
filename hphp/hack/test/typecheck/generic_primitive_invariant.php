<?hh // partial

/*
 * We should be able to inline addone and get the same error;
 * however, due to a TODO for arithmetic operators in Typing, this does not work
 *
 * See the TODO in Ast.Minus | Ast.Star in typing.ml for more details
 */

function addone(int $x): int {
  return $x + 1;
}

function no<T as int>(T $i): T {
  return addone($i);
}
