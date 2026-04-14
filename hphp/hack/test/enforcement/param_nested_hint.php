<?hh

// Fix 1: Nested type arguments are NOT at enforced boundaries.
// The int inside Ref<int> should not report as enforced — only the
// outer Ref<int> type hint is at the parameter boundary.
function f(HH\Lib\Ref<int> $r): void {}
//                    ^ enforcement-at-caret
