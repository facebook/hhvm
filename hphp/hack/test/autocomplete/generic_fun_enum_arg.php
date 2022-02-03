<?hh

enum MyEnum: string {
  TYPE_A = "A value";
  TYPE_B = "B value";
  TYPE_C = "C value";
}

function takes_vec(Vector<MyEnum> $_): void {}

// Invariant, so we solve to Vector<MyEnum> at the call site.
function creates_vec<T>(T $x): Vector<T> { return Vector {$x}; }

function foo(): void {
  takes_vec(creates_vec(AUTO332));
}
