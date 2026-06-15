<?hh

// Without `--typed-open-shapes`, typed open shapes (with an explicit
// unknown-fields hint) must be rejected during naming, before typechecking
// runs. Plain `...` and `mixed...` remain allowed.

function takes_typed_open(shape('a' => int, string...) $s): void {}

type MyTypedOpen = shape('a' => int, string...);

class C {
  const type TX = shape('a' => int, string...);
}

// These should remain accepted regardless of the flag:
function takes_plain_open(shape('a' => int, ...) $s): void {}
function takes_mixed_open(shape('a' => int, mixed...) $s): void {}
