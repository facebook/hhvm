<?hh

// Large int literals (outside OCaml's 63-bit `int`) are recorded via the
// EMVLargeInt slow path and are still checked for duplicates -- previously they
// recorded no value and duplicates among them were silently missed. The slow
// path stores the literal as written and does not canonicalise, so only
// identical spellings compare equal (see LargeNotNormalized below).

enum LargeDup: int {
  A = 9223372036854775807; // i64::MAX
  B = 9223372036854775807; // same literal -> duplicate of A, flagged
}

enum LargeDistinct: int {
  Max = 9223372036854775807; // i64::MAX
  Big = 4611686018427387904; // 2^62 -> distinct, no error
}

// Accepted incompleteness: large literals are not canonicalised, so a hex and a
// decimal spelling of the same value are NOT detected as duplicates (no error).
enum LargeNotNormalized: int {
  C = 9223372036854775807; // i64::MAX in decimal
  D = 0x7FFFFFFFFFFFFFFF; // same value in hex -> not flagged (stored as written)
}
