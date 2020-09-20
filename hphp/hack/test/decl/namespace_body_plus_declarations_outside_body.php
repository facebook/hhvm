<?hh

namespace X {
  function a(): void {}
}

// This is illegal--can't mix namespace declarations with bodies and
// declarations outside a namespace body in the same file.
function b(): void {}
