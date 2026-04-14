<?hh

// Fix 4: nonnull is an enforceable bound and should be reported.
function foo<T as nonnull>(T $param): void {}
//                         ^ enforcement-at-caret
