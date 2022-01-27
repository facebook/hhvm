<?hh

function foo<Tfoo>(): void {
  $f = (Tfoo $x) ==> $x;
  //     ^ hover-at-caret
}
