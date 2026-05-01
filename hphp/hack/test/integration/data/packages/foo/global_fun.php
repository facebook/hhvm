<?hh

// Global function with the same short name as BarNs\shared_fun.
// Tests that the package lint correctly distinguishes \shared_fun
// from \BarNs\shared_fun.
function shared_fun(): void {}
