<?hh

namespace {
  function a(): void {}
}

// This is illegal--a file containing a global `namespace {}` declaration cannot
// have any code outside the namespace body.
function b(): void {}
