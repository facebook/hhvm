<?hh

// This tests the logic for finding relevant quickfixes for a range.
// The tests in //hphp/hack/test/quickfixes exercise quickfixes more.
function f(int $x): void {
    =/*range-start*/$x/*range-end*/;
}
