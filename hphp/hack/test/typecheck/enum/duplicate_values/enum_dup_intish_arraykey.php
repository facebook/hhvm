<?hh

// `arraykey` enums coerce "intish" string values to ints when `getNames()`
// builds its value->name map, so an int and the canonical string form of the
// same number collide at runtime (getNames() throws). The duplicate-value check
// canonicalizes intish strings to catch this.

enum DemoIntishString: arraykey {
  INT = 0;
  STRING = '0'; // '0' coerces to int 0 -> duplicate of INT, flagged
}

// Two intish strings equal to an int also collide.
enum TwoIntish: arraykey {
  X = '5';
  Y = 5; // 5 == '5' -> flagged
}

// A large int (outside OCaml's 63-bit `int`) is recorded as its canonical
// decimal, so it collides with the equivalent intish string whatever spelling
// the literal uses.
enum LargeIntishCollision: arraykey {
  HEX_INT = 0x4000000000000000;
  DECIMAL_STRING = '4611686018427387904'; // same value -> flagged
}

// Negative cases: non-canonical strings are NOT intish, so no coercion and no
// collision with the int `0`.
enum NotIntish: arraykey {
  Zero = 0;
  DoubleZero = '00'; // not canonical -> stays a distinct string
  NegZero = '-0'; // not canonical -> distinct
  Hex = '0x0'; // hex string -> distinct
}
