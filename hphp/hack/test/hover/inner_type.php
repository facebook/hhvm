<?hh

function foo(): ?vec<int> {
  //                 ^ hover-at-caret
  return vec[];
}
