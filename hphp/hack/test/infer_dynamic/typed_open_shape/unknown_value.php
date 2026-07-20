<?hh

function takes_int(int $_): void {}

// Unknown fields are typed int, so reading one via Shapes::at yields int.
function reads_unknown_field(shape('a' => int, int...) $s): void {
  takes_int(Shapes::at($s, 'unknown'));
}

// Dynamic constrained to a shape with a typed tail: the solver reports the
// typed tail (int...) in the solution.
function test_typed_tail(dynamic $d): void {
  reads_unknown_field($d);
}

function expects_dynamic_tail(shape('a' => int, dynamic...) $_): void {}

// Dynamic constrained to a shape with a dynamic tail: the solver resolves the
// tail instead of leaking a raw dynamic into the solution.
function test_dynamic_tail(dynamic $d): void {
  expects_dynamic_tail($d);
}
