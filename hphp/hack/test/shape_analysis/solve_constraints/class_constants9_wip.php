<?hh
// T134628593
class C {
  const dict<string, mixed> DICT = dict['a' => 42];
}

class D extends C {
  const dict<string, mixed> DICT = dict['a' => 42];
  // Incorrectly solved to:
  // const shape('a' => int) DICT = shape('a' => 42);
  // Should be:
  // const shape('a' => int, ?'b' => mixed) DICT = shape('a' => 42);
}

class E extends D {
  // Incorrectly solved to:
  // const shape(?'b' => mixed) DICT = shape('b' => .......);
  // Should not be any solution for E.
}

function main(): void {
    idx(E::DICT, 'b');
}
