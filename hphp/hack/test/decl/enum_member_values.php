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
  Under = 1_6;
  Neg = -5;
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
