<?hh

// Plain enums record each member's canonical value in the shallow decl
// (`scc_value`), which a later check uses to flag duplicate enum values. Enum
// classes are intentionally excluded, so they keep `scc_value = None`.

enum Bits: int {
  Zero = 0;
  One = 1;
  AlsoOne = 1; // same value as One, on purpose, to show the captured values
}

enum MoreBits: int {
  use Bits;
  Two = 2;
}

enum Letters: string {
  A = 'a';
  B = 'b';
}

// Integer literal forms all canonicalize to plain decimal: Dec, Hex, Bin, Oct
// (leading-zero octal) and Under are all 16.
enum IntForms: int {
  Dec = 16;
  Hex = 0x10;
  Bin = 0b10000;
  Oct = 020;
  Neg = -5;
}

// Values beyond OCaml's 63-bit `int` use the EMVLargeInt slow path, storing the
// literal as written (not canonicalised), so i64::MIN aside, large values are
// still recorded. Different spellings of the same large value are not
// normalised. Values within 63 bits stay EMVInt.
enum BigInts: int {
  Fits = 4611686018427387903; // 2^62 - 1 = OCaml max_int -> EMVInt
  Big = 4611686018427387904; // 2^62 -> EMVLargeInt "4611686018427387904"
  BigHex = 0x4000000000000001; // 2^62 + 1 -> EMVLargeInt "0x4000000000000001" (raw)
  Max = 9223372036854775807; // i64::MAX -> EMVLargeInt "9223372036854775807"
}

class C {}

// nameof resolves to the class name string.
enum StrForms: string {
  Plain = 'hi';
  Named = nameof C;
}

// Computed values (constant references, arithmetic) are not captured.
const int GLOBAL_FIVE = 5;

enum Computed: int {
  FromConst = GLOBAL_FIVE;
  FromExpr = 2 + 3;
}
