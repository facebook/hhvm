<?hh

// Large int literals (outside OCaml's 63-bit `int`) are recorded via the
// EMVLargeInt slow path and are still checked for duplicates -- previously they
// recorded no value and duplicates among them were silently missed. The slow
// path stores the canonical decimal, so spellings of one value compare equal
// (see LargeNormalized below).

enum LargeDup: int {
  A = 9223372036854775807; // i64::MAX
  B = 9223372036854775807; // same literal -> duplicate of A, flagged
}

enum LargeDistinct: int {
  Max = 9223372036854775807; // i64::MAX
  Big = 4611686018427387904; // 2^62 -> distinct, no error
}

// Spellings are canonicalised, so hex and decimal forms of one value collide.
enum LargeNormalized: int {
  C = 9223372036854775807; // i64::MAX in decimal
  D = 0x7FFFFFFFFFFFFFFF; // same value in hex -> duplicate of C, flagged
}
