<?hh

function returns_generic<T>(T $x): T {
  return $x;
//       ^ enforcement-at-caret
}
