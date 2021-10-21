<?hh

// Using shape() because shapes are not considered enforceable, so
// $s will be dynamic in the SDT pass instead of (dynamic & shape())
<<__SupportDynamicType>>
function f(shape() $s): void {
  g($s);
}

function g(shape() $s): void {}
