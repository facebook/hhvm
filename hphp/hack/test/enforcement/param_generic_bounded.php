<?hh

function foo<T as arraykey>(T $x): void {}
//                          ^ enforcement-at-caret
