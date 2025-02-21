<?hh

final class CGeneric<T> {}

function f(): void {
  $c = new CGeneric();
//         ^ hover-at-caret
}
