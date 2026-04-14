<?hh

// Fix 2: Generics bounded by effectively-enforced container types
// should report the bound as enforced.
function foo<T as vec<mixed>>(T $param): void {}
//                            ^ enforcement-at-caret
