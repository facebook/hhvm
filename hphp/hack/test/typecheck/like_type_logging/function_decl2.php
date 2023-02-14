<?hh

// The following should have a like type in its parameter as it is effectively
// returning a `vec<int>` which would get pessimised.
// This currently doesn't happen due to T143986620
function f(inout vec<int> $_): void {}
