<?hh

function foo<Tfoo as int>(): Tfoo {
  //                         ^ hover-at-caret
  throw new Exception('');
}
